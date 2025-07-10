#include "protocol.h"
#include "utils.h"
#include <stdint.h>
#include <sys/socket.h>

int protocol_recv_command(int fd, char *buf, size_t len) {

  char *p = buf;
  size_t bytes_read = 0;
  do {

    if (bytes_read >= 255) {

      fprintf(stderr, "[unotd:server] ERROR: command too large\n");
      send(fd, "ERROR\n\n", 7, 0);
      return 1;
    }

    ssize_t n = recv(fd, buf + bytes_read, len - bytes_read - 1, 0);

    if (n <= 0) {

      if (n < 0) {
        perror("[unotd:server] ERROR: recv");
      }

      fprintf(stderr, "[unotd:server] INFO: connection closed by the client\n");
      return 1;
    }

    if (bytes_read != 0)
      p = buf + bytes_read - 1;

    bytes_read += n;

  } while (strstr(p, "\n\n") == NULL);

  return 0;
}

static bool _parse_ntf_fields(Unotd *unotd, Notification *target) {

  const char *txt = NULL;
  const char *ind = NULL;
  bool got_type = false;

  char *pair = NULL;
  while ((pair = strtok(NULL, "\n"))) {

    char *colon = strchr(pair, ':');
    if (!colon) {
      fprintf(stderr, "[unotd:server] ERROR: malformed command: missing ':'\n");
      return false;
    }
    *colon = '\0';

    char *key = pair;
    char *val = colon + 1;
    if (!key || !val) {
      fprintf(stderr, "[unotd:server] ERROR: malformed key-value pair\n");
      return false;
    }

    if (strncmp(key, "txt", 3) == 0)
      txt = val;

    else if (strncmp(key, "typ", 3) == 0) {

      if (strncmp(val, "msg", 3) == 0)
        target->type = UNOT_TYPE_MESSAGE;
      else if (strncmp(val, "spn", 3) == 0)
        target->type = UNOT_TYPE_SPINNER;
      else {
        fprintf(stderr,
                "[unotd:server] ERROR: unknown notification type: '%s'\n", val);
        return false;
      }

      got_type = true;
    }

    else if (strncmp(key, "ind", 3) == 0)
      ind = val;

    else if (strncmp(key, "nme", 3) == 0) {
      for (size_t i = 0; i < unotd->config.indicators_count; i++) {
        char *indicator = unotd->config.indicators[i];
        if (strncmp(val, indicator, strlen(val)) == 0) {
          target->indicator = indicator + strlen(indicator) + 1;
          break;
        }
      }
      if (!target->indicator)
        fprintf(stderr,
                "[unotd:server] WARN: indicator of name '%s' not found\n", val);
    }

    else if (strncmp(key, "tfn", 3) == 0)
      target->txt_font = (void *)val;

    else if (strncmp(key, "tfg", 3) == 0)
      target->txt_color = (void *)val;

    else if (strncmp(key, "ifg", 3) == 0)
      target->ind_color = (void *)val;

    else if (strncmp(key, "tim", 3) == 0) {
      if (!utils_parse_ul(val, &target->timeout, 10)) {
        fprintf(stderr, "[unotd:server] ERROR: invalid timeout value: '%s'\n",
                val);
        return false;
      }
    }

    else {
      fprintf(stderr, "[unotd:server] ERROR: unknown key: '%s'\n", key);
      return false;
    }
  }

  if (!txt) {
    fprintf(stderr, "[unotd:server] ERROR: missing key: 'txt'\n");
    return false;
  }

  if (!got_type) {
    fprintf(stderr, "[unotd:server] ERROR: missing key: 'typ'\n");
    return false;
  }

  if (!ind && !target->indicator) {
    fprintf(stderr, "[unotd:server] ERROR: missing key: 'ind' or 'nme'\n");
    return false;
  }

  if (target->indicator)
    target->text = strdup(txt);

  else {
    size_t txt_len = strlen(txt) + 1;
    size_t ind_len = strlen(ind) + 1;
    target->text = malloc(txt_len + ind_len);
    memcpy(target->text, txt, txt_len);
    target->indicator = target->text + txt_len;
    memcpy(target->indicator, ind, ind_len);
  }

  return true;
}

static bool _parse_ret_fields(unsigned long *wid, unsigned long *ret) {

  bool got_wid = false;
  bool got_ret = false;

  char *pair = NULL;
  while ((pair = strtok(NULL, "\n"))) {

    char *colon = strchr(pair, ':');
    if (!colon) {
      fprintf(stderr, "[unotd:server] ERROR: malformed command: missing ':'\n");
      return false;
    }
    *colon = '\0';

    char *key = pair;
    char *val = colon + 1;
    if (!key || !val) {
      fprintf(stderr, "[unotd:server] ERROR: malformed key-value pair\n");
      return false;
    }

    if (strncmp(key, "wid", 3) == 0) {
      if (!utils_parse_ul(val, wid, 10)) {
        fprintf(stderr, "[unotd:server] ERROR: invalid notification id: '%s'\n",
                val);
        return false;
      }
      got_wid = true;
    }

    else if (strncmp(key, "ret", 3) == 0) {
      if (!utils_parse_ul(val, ret, 10)) {
        fprintf(stderr, "[unotd:server] ERROR: invalid return value: '%s'\n",
                val);
        return false;
      }
      got_ret = true;
    }

    else {
      fprintf(stderr, "[unotd:server] ERROR: unknown key: '%s'\n", key);
      return false;
    }
  }

  if (!got_wid) {
    fprintf(stderr, "[unotd:server] ERROR: missing key: 'wid'\n");
    return false;
  }

  if (!got_ret) {
    fprintf(stderr, "[unotd:server] ERROR: missing key: 'ret'\n");
    return false;
  }

  return true;
}

void protocol_handle_command(Unotd *unotd, int fd, char *buf, size_t len) {

  char response[32] = {0};
  NotificationNode *node = NULL;

  const char *cmd = strtok(buf, "\n");
  if (cmd == NULL) {
    fprintf(stderr, "[unotd:server] ERROR: missing command\n");
    goto ERROR;
  }

  if (strncmp(cmd, "NTF", 3) == 0) {

    node = calloc(1, sizeof(NotificationNode));
    ASSERT(node && "protocol_handle_command: malloc failed");
    Notification *not = &node->notification;
    not->state = UNOT_NEED_INIT;
    not->timeout = unotd->config.timeout;

    if (!_parse_ntf_fields(unotd, &node->notification)) {
      free(not->text);
      free(node);
      goto ERROR;
    }

    pthread_mutex_lock(&unotd->nlist_lock);

    nlist_append(&unotd->open, node);
    pthread_cond_signal(&unotd->nlist_empty);

    while (node->notification.window == 0) {
      pthread_cond_wait(&unotd->notif_open, &unotd->nlist_lock);
    }

    pthread_mutex_unlock(&unotd->nlist_lock);

    size_t offset = 0;
    offset += snprintf(response + offset, 32 - offset, "OK\n");

    if (not->type == UNOT_TYPE_SPINNER)
      offset +=
          snprintf(response + offset, 32 - offset, "wid:%lu\n", not->window);

    offset += snprintf(response + offset, 32 - offset, "\n");
    goto OK;
  }

  else if (strncmp(cmd, "RET", 3) == 0) {

    unsigned long wid;
    unsigned long ret;
    if (!_parse_ret_fields(&wid, &ret))
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
        fprintf(stderr, "[unotd:server] ERROR: unknown notification id '%lu'\n",
                wid);
        pthread_mutex_unlock(&unotd->nlist_lock);
        goto ERROR;
      }
      target->notification.state = UNOT_NEED_REDRAW;
    }
    utils_transform_notification(&target->notification, ret);

    pthread_mutex_unlock(&unotd->nlist_lock);

    snprintf(response, 32, "OK\n\n");
    goto OK;
  }

  else {

    fprintf(stderr, "[unotd:server] ERROR: unknown command '%s'\n", cmd);
    goto ERROR;
  }

ERROR:
  snprintf(response, 32, "ERROR\n\n");

OK:
  send(fd, response, strlen(response), 0);
}
