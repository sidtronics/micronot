#include "command.h"
#include "../protocol.h"
#include "utils.h"
#include <sys/socket.h>

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

    if (strcmp(key, UNOT_KEY_TEXT) == 0)
      txt = val;

    else if (strcmp(key, UNOT_KEY_TYPE) == 0) {

      if (strcmp(val, UNOT_VAL_MESSAGE) == 0)
        target->type = UNOT_TYPE_MESSAGE;
      else if (strcmp(val, UNOT_VAL_SPINNER) == 0)
        target->type = UNOT_TYPE_SPINNER;
      else {
        fprintf(stderr,
                "[unotd:server] ERROR: unknown notification type: '%s'\n", val);
        return false;
      }

      got_type = true;
    }

    else if (strcmp(key, UNOT_KEY_INDICATOR) == 0)
      ind = val;

    else if (strcmp(key, UNOT_KEY_INDICATOR_NAME) == 0) {
      for (size_t i = 0; i < unotd->config.indicators_count; i++) {
        char *indicator = unotd->config.indicators[i];
        if (strcmp(val, indicator) == 0) {
          target->indicator = indicator + strlen(indicator) + 1;
          break;
        }
      }
      if (!target->indicator)
        fprintf(stderr,
                "[unotd:server] WARN: indicator of name '%s' not found\n", val);
    }

    else if (strcmp(key, UNOT_KEY_TEXT_FONT) == 0)
      target->txt_font = (void *)val;

    else if (strcmp(key, UNOT_KEY_TEXT_FG) == 0)
      target->txt_color = (void *)val;

    else if (strcmp(key, UNOT_KEY_INDICATOR_FG) == 0)
      target->ind_color = (void *)val;

    else if (strcmp(key, UNOT_KEY_TIMEOUT) == 0) {
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
    fprintf(stderr, "[unotd:server] ERROR: missing key: '%s'\n", UNOT_KEY_TEXT);
    return false;
  }

  if (!got_type) {
    fprintf(stderr, "[unotd:server] ERROR: missing key: '%s'\n", UNOT_KEY_TYPE);
    return false;
  }

  if (!ind && !target->indicator) {
    fprintf(stderr, "[unotd:server] ERROR: missing key: '%s' or '%s'\n",
            UNOT_KEY_INDICATOR, UNOT_KEY_INDICATOR_NAME);
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

    if (strcmp(key, UNOT_KEY_NOTIF_ID) == 0) {
      if (!utils_parse_ul(val, wid, 10)) {
        fprintf(stderr, "[unotd:server] ERROR: invalid notification id: '%s'\n",
                val);
        return false;
      }
      got_wid = true;
    }

    else if (strcmp(key, UNOT_KEY_RETURN_VALUE) == 0) {
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
    fprintf(stderr, "[unotd:server] ERROR: missing key: '%s'\n",
            UNOT_KEY_NOTIF_ID);
    return false;
  }

  if (!got_ret) {
    fprintf(stderr, "[unotd:server] ERROR: missing key: '%s'\n",
            UNOT_KEY_RETURN_VALUE);
    return false;
  }

  return true;
}

void command_handle(Unotd *unotd, int fd, char *buf, size_t len) {

  char response[32];
  NotificationNode *node = NULL;

  const char *cmd = strtok(buf, "\n");
  if (cmd == NULL) {
    fprintf(stderr, "[unotd:server] ERROR: missing command\n");
    goto ERROR;
  }

  if (strcmp(cmd, UNOT_CMD_NOTIFY) == 0) {

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
    offset += snprintf(response + offset, sizeof(response) - offset, "OK\n");

    if (not->type == UNOT_TYPE_SPINNER)
      offset += snprintf(response + offset, sizeof(response) - offset,
                         "%s:%lu\n", UNOT_KEY_NOTIF_ID, not->window);

    offset += snprintf(response + offset, sizeof(response) - offset, "\n");
    goto OK;
  }

  else if (strcmp(cmd, UNOT_CMD_RETURN) == 0) {

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

    snprintf(response, sizeof(response), "OK\n\n");
    goto OK;
  }

  else {

    fprintf(stderr, "[unotd:server] ERROR: unknown command '%s'\n", cmd);
    goto ERROR;
  }

ERROR:
  snprintf(response, sizeof(response), "ERROR\n\n");

OK:
  protocol_send_block(fd, response, sizeof(response));
}
