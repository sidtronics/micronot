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

static int _parse_fields(Notification *target) {

  const char *msg = NULL;
  const char *ind = NULL;

  while (1) {

    const char *field = strtok(NULL, ":");

    if (strcmp(field, "\n") == 0)
      break;

    const char *value = strtok(NULL, "\n");
    if (value == NULL) {
      fprintf(stderr, "error: malformed field: \"%s\"\n", field);
      return 1;
    }

    if (strcmp(field, "msg") == 0)
      msg = value;

    else if (strcmp(field, "ind") == 0)
      ind = value;

    else if (strcmp(field, "nme") == 0) {
      // TODO
    }

    else if (strcmp(field, "mfn") == 0)
      target->msg_font = (void *)strdup(value);

    else if (strcmp(field, "mfg") == 0)
      target->msg_color = (void *)strdup(value);

    else if (strcmp(field, "ifg") == 0)
      target->ind_color = (void *)strdup(value);

    else if (strcmp(field, "tmo") == 0) {

      char *end;
      unsigned long timeout_ul = strtoul(value, &end, 10);
      if (value[0] == '-' || *end != '\0') {
        fprintf(stderr, "error: invalid timeout\n");
        return 1;
      }

      if (timeout_ul > INT_MAX) {
        fprintf(stderr, "error: timeout out of range\n");
        return 1;
      }

      target->timeout = (int)timeout_ul;
    }

    else {

      fprintf(stderr, "error: unknown field: \"%s\"\n", field);
      return 1;
    }
  }

  if (msg == NULL) {

    fprintf(stderr, "error: message not found");
    return 1;
  }

  if (ind == NULL) {

    fprintf(stderr, "error: indicator not found");
    return 1;
  }

  size_t msg_len = strlen(msg) + 1;
  size_t ind_len = strlen(ind) + 1;
  size_t len = msg_len + ind_len;
  target->message = malloc(len);
  memcpy(target->message, msg, msg_len);
  target->indicator = target->message + msg_len;
  memcpy(target->indicator, ind, ind_len);
  return 0;
}

void protocol_handle_command(Unotd *unotd, int fd, char *buf, size_t len) {

  char response[32] = {0};
  NotificationNode *node = NULL;

  const char *cmd = strtok(buf, "\n");
  if (cmd == NULL || strlen(cmd) < 3) {
    fprintf(stderr, "error: invalid command\n");
    goto ERROR;
  }

  if (strncmp(cmd, "MSG", 3) == 0) {

    node = malloc(sizeof(NotificationNode));
    ASSERT(node && "protocol_handle_command: malloc failed");
    Notification *not = &node->notification;
    not->window = 0;
    not->msg_font = NULL;
    not->msg_color = NULL;
    not->ind_color = NULL;
    not->type = UNOT_TYPE_MESSAGE;
    not->state = UNOT_STATE_UNMAPPED;
    not->timeout = unotd->config.timeout;

    if (_parse_fields(&node->notification) != 0)
      goto ERROR;

    notification_list_append(&unotd->open, node);

    sem_post(&unotd->notification_count);

    snprintf(response, 32, "OK\n\n");
    goto OK;
  }

  else if (strncmp(cmd, "SPN", 3) == 0) {

    node = malloc(sizeof(NotificationNode));
    ASSERT(node && "protocol_handle_command: malloc failed");
    Notification *not = &node->notification;
    not->window = 0;
    not->msg_font = NULL;
    not->msg_color = NULL;
    not->ind_color = NULL;
    not->type = UNOT_TYPE_SPINNER;
    not->state = UNOT_STATE_UNMAPPED;
    not->timeout = unotd->config.timeout;

    if (_parse_fields(&node->notification) != 0)
      goto ERROR;

    notification_list_append(&unotd->open, node);

    sem_post(&unotd->notification_count);

    pthread_mutex_lock(&unotd->open.lock);
    while (node->notification.window == 0) {
      pthread_cond_wait(&unotd->notification_opened, &unotd->open.lock);
    }
    pthread_mutex_unlock(&unotd->open.lock);

    snprintf(response, 32, "OK\nwid:%lu\n\n", node->notification.window);
    goto OK;
  }

  else if (strncmp(cmd, "RET", 3) == 0) {

    const char *wid_str = strtok(NULL, "\n");
    if (wid_str == NULL) {
      fprintf(stderr, "error: window id not found\n");
      goto ERROR;
    }

    const char *ret_str = strtok(NULL, "\n");
    if (wid_str == NULL) {
      fprintf(stderr, "error: return value not found\n");
      goto ERROR;
    }

    char *end;
    unsigned long wid = strtoul(wid_str, &end, 10);
    if (*end != '\0') {
      fprintf(stderr, "error: invalid window id: \"%s\"\n", wid_str);
      goto ERROR;
    }

    unsigned long ret = strtoul(ret_str, &end, 10);
    if (*end != '\0') {
      fprintf(stderr, "error: invalid return value: \"%s\"\n", ret_str);
      goto ERROR;
    }

    NotificationNode *prev = NULL;
    NotificationNode *target = notification_list_find(
        &unotd->open, &prev, utils_match_window, (void *)(uintptr_t)wid);

    pthread_mutex_lock(&unotd->open.lock);
    if (target) {
      utils_transform_notification(&target->notification, ret);
      target->notification.state = UNOT_STATE_UNMAPPED; 
      pthread_mutex_unlock(&unotd->open.lock);
    }

    else {

      pthread_mutex_unlock(&unotd->open.lock);
      target = notification_list_find(&unotd->wait, &prev, utils_match_window,
                                      (void *)(uintptr_t)wid);

      if (target == NULL) {
        fprintf(stderr, "error: unknown window id: %lu\n", wid);
        goto ERROR;
      }

      notification_list_unlink(&unotd->wait, prev);
      utils_transform_notification(&target->notification, ret);
      notification_list_append(&unotd->open, target);

      sem_post(&unotd->notification_count);
    }

    snprintf(response, 32, "OK\n\n");
    goto OK;
  }

  else {

    fprintf(stderr, "error: unknown command\n");
    goto ERROR;
  }

ERROR:
  if (node != NULL) {

    free((void *)node->notification.message);
    free(node->notification.msg_font);
    free(node->notification.msg_color);
    free(node->notification.ind_color);
    free(node);
  }
  snprintf(response, 32, "ERROR\n\n");

OK:
  send(fd, response, strlen(response), 0);
}
