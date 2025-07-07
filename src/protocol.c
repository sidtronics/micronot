#include "protocol.h"
#include "utils.h"
#include <stdint.h>
#include <sys/socket.h>

int protocol_recv_command(int fd, char *buf, size_t len) {

  char *p = buf;
  size_t bytes_read = 0;
  do {

    if (bytes_read >= 255) {

      fprintf(stderr, "error: buffer overflow\n");
      send(fd, "ERROR\n\n", 7, 0);
      return 1;
    }

    ssize_t n = recv(fd, buf + bytes_read, len - bytes_read - 1, 0);

    if (n <= 0) {

      if (n < 0) {
        perror("recv");
      }

      fprintf(stderr, "info: client closed the connection\n");
      return 1;
    }

    if (bytes_read != 0)
      p = buf + bytes_read - 1;

    bytes_read += n;

  } while (strstr(p, "\n\n") == NULL);

  return 0;
}

static bool _parse_fields(Notification *target) {

  const char *txt = NULL;
  const char *ind = NULL;

  while (1) {

    const char *field = strtok(NULL, ":");

    if (strncmp(field, "\n", 1) == 0)
      break;

    const char *value = strtok(NULL, "\n");
    if (value == NULL) {
      fprintf(stderr, "error: malformed field\n");
      return 1;
    }

    if (strncmp(field, "txt", 3) == 0)
      txt = value;

    else if (strncmp(field, "ind", 3) == 0)
      ind = value;

    else if (strncmp(field, "nme", 3) == 0) {
      // TODO
    }

    else if (strncmp(field, "tfn", 3) == 0)
      target->txt_font = (void *)value;

    else if (strncmp(field, "tfg", 3) == 0)
      target->txt_color = (void *)value;

    else if (strncmp(field, "ifg", 3) == 0)
      target->ind_color = (void *)value;

    else if (strncmp(field, "tim", 3) == 0) {
      if (!utils_parse_ul(value, &target->timeout, 10)) {
        fprintf(stderr, "error: invalid timeout\n");
        return 1;
      }
    }

    else {
      fprintf(stderr, "error: unknown field: \"%s\"\n", field);
      return 1;
    }
  }

  if (txt == NULL) {
    fprintf(stderr, "error: text not set");
    return 1;
  }

  if (ind == NULL) {
    fprintf(stderr, "error: indicator not set");
    return 1;
  }

  size_t txt_len = strlen(txt) + 1;
  size_t ind_len = strlen(ind) + 1;
  target->text = malloc(txt_len + ind_len);
  memcpy(target->text, txt, txt_len);
  target->indicator = target->text + txt_len;
  memcpy(target->indicator, ind, ind_len);
  return 0;
}

static bool _parse_ret_fields(unsigned long *wid, unsigned long *ret) {

  bool wid_set = false;
  bool ret_set = false;

  while (1) {

    const char *field = strtok(NULL, ":");

    if (strncmp(field, "\n", 1) == 0)
      break;

    const char *value = strtok(NULL, "\n");
    if (value == NULL) {
      fprintf(stderr, "error: malformed field\n");
      return 1;
    }

    if (strncmp(field, "wid", 3) == 0) {
      if (!utils_parse_ul(value, wid, 10)) {
        fprintf(stderr, "error: invalid window id\n");
        return 1;
      }
      wid_set = true;
    }

    else if (strncmp(field, "ret", 3) == 0) {
      if (!utils_parse_ul(value, ret, 10)) {
        fprintf(stderr, "error: invalid return value\n");
        return 1;
      }
      ret_set = true;
    }

    else {
      fprintf(stderr, "error: unknown field: \"%s\"\n", field);
      return 1;
    }
  }

  if (!wid_set) {
    fprintf(stderr, "error: window id not set\n");
    return 1;
  }

  if (!ret_set) {
    fprintf(stderr, "error: window id not set\n");
    return 1;
  }

  return 0;
}

void protocol_handle_command(Unotd *unotd, int fd, char *buf, size_t len) {

  char response[32] = {0};
  NotificationNode *node = NULL;

  const char *cmd = strtok(buf, "\n");
  if (cmd == NULL || strlen(cmd) < 3) {
    fprintf(stderr, "error: invalid command: \"%s\"\n", cmd);
    goto ERROR;
  }

  if (strncmp(cmd, "MSG", 3) == 0) {

    node = calloc(1, sizeof(NotificationNode));
    ASSERT(node && "protocol_handle_command: malloc failed");
    Notification *not = &node->notification;
    not->type = UNOT_TYPE_MESSAGE;
    not->state = UNOT_NEED_INIT;
    not->timeout = unotd->config.timeout;

    if (_parse_fields(&node->notification) != 0)
      goto ERROR;

    pthread_mutex_lock(&unotd->nlist_lock);

    nlist_append(&unotd->open, node);
    pthread_cond_signal(&unotd->nlist_empty);

    while (node->notification.window == 0) {
      pthread_cond_wait(&unotd->notif_open, &unotd->nlist_lock);
    }

    pthread_mutex_unlock(&unotd->nlist_lock);

    snprintf(response, 32, "OK\n\n");
    goto OK;
  }

  else if (strncmp(cmd, "SPN", 3) == 0) {

    node = malloc(sizeof(NotificationNode));
    ASSERT(node && "protocol_handle_command: malloc failed");
    Notification *not = &node->notification;
    not->type = UNOT_TYPE_SPINNER;
    not->state = UNOT_NEED_INIT;
    not->timeout = unotd->config.timeout;

    if (_parse_fields(&node->notification) != 0)
      goto ERROR;

    pthread_mutex_lock(&unotd->nlist_lock);

    nlist_append(&unotd->open, node);
    pthread_cond_signal(&unotd->nlist_empty);

    while (node->notification.window == 0) {
      pthread_cond_wait(&unotd->notif_open, &unotd->nlist_lock);
    }

    pthread_mutex_unlock(&unotd->nlist_lock);

    snprintf(response, 32, "OK\nwid:%lu\n\n", node->notification.window);
    goto OK;
  }

  else if (strncmp(cmd, "RET", 3) == 0) {

    unsigned long wid;
    unsigned long ret;
    if (_parse_ret_fields(&wid, &ret) != 0)
      goto ERROR;

    pthread_mutex_lock(&unotd->nlist_lock);

    NotificationNode *prev = NULL;
    NotificationNode *target = nlist_find(&unotd->wait, &prev, wid);

    if (target) {
      target->notification.state = UNOT_NEED_REOPEN;
      nlist_unlink(&unotd->wait, prev);
      nlist_append(&unotd->open, target);
      pthread_cond_signal(&unotd->nlist_empty);
    }

    else {
      target = nlist_find(&unotd->open, &prev, wid);
      if (!target) {
        fprintf(stderr, "error: unknown window id: %lu\n", wid);
        pthread_mutex_unlock(&unotd->nlist_lock);
        goto ERROR;
      }
      target->notification.state = UNOT_NEED_REDRAW;
    }

    utils_transform_notification(&target->notification, ret);
    snprintf(response, 32, "OK\n\n");
    pthread_mutex_unlock(&unotd->nlist_lock);
    goto OK;
  }

  else {

    fprintf(stderr, "error: unknown command\n");
    goto ERROR;
  }

ERROR:
  if (node != NULL) {
    free((void *)node->notification.text);
    free(node);
  }
  snprintf(response, 32, "ERROR\n\n");

OK:
  send(fd, response, strlen(response), 0);
}
