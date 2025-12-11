#include "command.h"
#include "../protocol.h"
#include "utils.h"
#include <pthread.h>

// static bool _parse_ntf_fields(Unotd *unotd, Notification *target) {
//
//   const char *txt = NULL;
//   const char *ind = NULL;
//   bool got_type = false;
//
//   char *pair = NULL;
//   while ((pair = strtok(NULL, "\n"))) {
//
//     char *colon = strchr(pair, ':');
//     if (!colon) {
//       fprintf(stderr, "[unotd:server] ERROR: malformed command: missing
//       ':'\n"); return false;
//     }
//
//     *colon = '\0';
//
//     char *key = pair;
//     char *val = colon + 1;
//     if (!key || !val) {
//       fprintf(stderr, "[unotd:server] ERROR: malformed key-value pair\n");
//       return false;
//     }
//
//     if (strcmp(key, UNOT_KEY_TEXT) == 0)
//       txt = val;
//
//     else if (strcmp(key, UNOT_KEY_TYPE) == 0) {
//
//       if (strcmp(val, UNOT_VAL_MESSAGE) == 0)
//         target->type = UNOT_TYPE_MESSAGE;
//       else if (strcmp(val, UNOT_VAL_SPINNER) == 0)
//         target->type = UNOT_TYPE_SPINNER;
//       else {
//         fprintf(stderr,
//                 "[unotd:server] ERROR: unknown notification type: '%s'\n",
//                 val);
//         return false;
//       }
//
//       got_type = true;
//     }
//
//     else if (strcmp(key, UNOT_KEY_INDICATOR) == 0)
//       ind = val;
//
//     else if (strcmp(key, UNOT_KEY_INDICATOR_NAME) == 0) {
//       for (size_t i = 0; i < unotd->config.indicators_count; i++) {
//         char *indicator = unotd->config.indicators[i];
//         if (strcmp(val, indicator) == 0) {
//           target->indicator = indicator + strlen(indicator) + 1;
//           break;
//         }
//       }
//       if (!target->indicator)
//         fprintf(stderr,
//                 "[unotd:server] WARN: indicator of name '%s' not found\n",
//                 val);
//     }
//
//     else if (strcmp(key, UNOT_KEY_TEXT_FONT) == 0)
//       target->txt_font = (void *)val;
//
//     else if (strcmp(key, UNOT_KEY_TEXT_FG) == 0)
//       target->txt_color = (void *)val;
//
//     else if (strcmp(key, UNOT_KEY_INDICATOR_FG) == 0)
//       target->ind_color = (void *)val;
//
//     else if (strcmp(key, UNOT_KEY_TIMEOUT) == 0) {
//       if (!utils_parse_ul(val, &target->timeout, 10)) {
//         fprintf(stderr, "[unotd:server] ERROR: invalid timeout value:
//         '%s'\n",
//                 val);
//         return false;
//       }
//     }
//
//     else {
//       fprintf(stderr, "[unotd:server] ERROR: unknown key: '%s'\n", key);
//       return false;
//     }
//   }
//
//   if (!txt) {
//     fprintf(stderr, "[unotd:server] ERROR: missing key: '%s'\n",
//     UNOT_KEY_TEXT); return false;
//   }
//
//   if (!got_type) {
//     fprintf(stderr, "[unotd:server] ERROR: missing key: '%s'\n",
//     UNOT_KEY_TYPE); return false;
//   }
//
//   if (!ind && !target->indicator) {
//     fprintf(stderr, "[unotd:server] ERROR: missing key: '%s' or '%s'\n",
//             UNOT_KEY_INDICATOR, UNOT_KEY_INDICATOR_NAME);
//     return false;
//   }
//
//   if (target->indicator)
//     target->text = strdup(txt);
//
//   else {
//     size_t txt_len = strlen(txt) + 1;
//     size_t ind_len = strlen(ind) + 1;
//     target->text = malloc(txt_len + ind_len);
//     memcpy(target->text, txt, txt_len);
//     target->indicator = target->text + txt_len;
//     memcpy(target->indicator, ind, ind_len);
//   }
//
//   if (!utils_validate_indicator(target->indicator, target->type)) {
//     fprintf(stderr, "[unotd:server] ERROR: malformed indicator string\n");
//     return false;
//   }
//
//   return true;
// }

// static bool _parse_ret_fields(unsigned long *wid, unsigned long *ret) {
//
//   bool got_wid = false;
//   bool got_ret = false;
//
//   char *pair = NULL;
//   while ((pair = strtok(NULL, "\n"))) {
//
//     char *colon = strchr(pair, ':');
//     if (!colon) {
//       fprintf(stderr, "[unotd:server] ERROR: malformed command: missing
//       ':'\n"); return false;
//     }
//
//     *colon = '\0';
//
//     char *key = pair;
//     char *val = colon + 1;
//     if (!key || !val) {
//       fprintf(stderr, "[unotd:server] ERROR: malformed key-value pair\n");
//       return false;
//     }
//
//     if (strcmp(key, UNOT_KEY_NOTIF_ID) == 0) {
//       if (!utils_parse_ul(val, wid, 10)) {
//         fprintf(stderr, "[unotd:server] ERROR: invalid notification id:
//         '%s'\n",
//                 val);
//         return false;
//       }
//       got_wid = true;
//     }
//
//     else if (strcmp(key, UNOT_KEY_RETURN_VALUE) == 0) {
//       if (!utils_parse_ul(val, ret, 10)) {
//         fprintf(stderr, "[unotd:server] ERROR: invalid return value: '%s'\n",
//                 val);
//         return false;
//       }
//       got_ret = true;
//     }
//
//     else {
//       fprintf(stderr, "[unotd:server] ERROR: unknown key: '%s'\n", key);
//       return false;
//     }
//   }
//
//   if (!got_wid) {
//     fprintf(stderr, "[unotd:server] ERROR: missing key: '%s'\n",
//             UNOT_KEY_NOTIF_ID);
//     return false;
//   }
//
//   if (!got_ret) {
//     fprintf(stderr, "[unotd:server] ERROR: missing key: '%s'\n",
//             UNOT_KEY_RETURN_VALUE);
//     return false;
//   }
//
//   return true;
// }

bool _parse_cmd_ntf(Unotd *unotd, ProtocolBuffer *pbuf, Notification *target) {

  const char *text = NULL;
  const char *indicator = NULL;

  ProtocolPair pair;
  while (protocol_parse(pbuf, &pair)) {

    if (!pair.key)
      return false;

    if (MATCH(pair.key, UNOT_KEY_TEXT))
      text = pair.val;

    else if (MATCH(pair.key, UNOT_KEY_INDICATOR))
      indicator = pair.val;

    else if (MATCH(pair.key, UNOT_KEY_INDICATOR_NAME)) {
      for (size_t i = 0; i < unotd->config.indicators_count; i++) {
        char *indicator = unotd->config.indicators[i];
        if (MATCH(pair.val, indicator)) {
          indicator_init(&target->ind, indicator + strlen(indicator) + 1);
          break;
        }
      }

      if (!target->ind.start)
        fprintf(stderr,
                "[unotd:server] WARN: indicator of name '%s' not found\n",
                pair.val);
    }

    else if (MATCH(pair.key, UNOT_KEY_TEXT_FONT))
      target->txt_font = XftFontOpenName(
          unotd->display, DefaultScreen(unotd->display), pair.val);

    else if (MATCH(pair.key, UNOT_KEY_TEXT_FG))
      target->txt_color =
          utils_allocate_color_s(unotd->display, pair.val, NULL);

    else if (MATCH(pair.key, UNOT_KEY_INDICATOR_FG))
      target->ind.color =
          utils_allocate_color_s(unotd->display, pair.val, NULL);

    else if (MATCH(pair.key, UNOT_KEY_TIMEOUT)) {
      if (!utils_parse_ul(pair.val, &target->timeout, 10)) {
        fprintf(stderr, "[unotd:server] ERROR: invalid timeout value: '%s'\n",
                pair.val);
        return false;
      }
    }

    else {
      fprintf(stderr, "[unotd:server] ERROR: unknown key: '%s'\n", pair.key);
      return false;
    }
  }

  if (!text) {
    fprintf(stderr, "[unotd:server] ERROR: missing key: '%s'\n", UNOT_KEY_TEXT);
    return false;
  }

  if (!indicator && !target->ind.start) {
    fprintf(stderr, "[unotd:server] ERROR: missing key: '%s' or '%s'\n",
            UNOT_KEY_INDICATOR, UNOT_KEY_INDICATOR_NAME);
    return false;
  }

  if (target->ind.start)
    target->txt = strdup(text);

  else {

    if (!indicator_validate(indicator)) {
      fprintf(stderr, "[unotd:server] ERROR: malformed indicator string\n");
      return false;
    }

    size_t txt_len = strlen(text) + 1;
    size_t ind_len = strlen(indicator) + 1;
    target->txt = malloc(txt_len + ind_len);
    memcpy(target->txt, text, txt_len);
    memcpy(target->txt + txt_len, indicator, ind_len);
    indicator_init(&target->ind, target->txt + txt_len);
  }

  indicator_resolve_font(unotd->display, &target->ind,
                         unotd->config.indicator_size);

  return true;
}

void command_handle(Unotd *unotd, int fd, ProtocolBuffer *pbuf) {

  NotificationNode *node = NULL;

  ProtocolPair pair;
  protocol_parse(pbuf, &pair);
  if (*pair.key == 0) {
    fprintf(stderr, "[unotd:server] ERROR: missing command\n");
    goto ERROR;
  }

  if (MATCH(pair.key, UNOT_CMD_NOTIFY)) {

    node = calloc(1, sizeof(NotificationNode));
    assert(node && "protocol_handle_command: malloc failed");
    Notification *not = &node->notification;
    not->txt_font = unotd->txt_font;
    not->txt_color = &unotd->txt_color;
    not->ind.color = &unotd->ind_color;
    not->timeout = unotd->config.timeout;
    not->state = UNOT_NEED_INIT;

    pthread_mutex_lock(&unotd->nlist_lock);

    if (!_parse_cmd_ntf(unotd, pbuf, not)) {
      free(not->txt);
      free(node);
      goto ERROR;
    }

    nlist_append(&unotd->open, node);
    pthread_cond_signal(&unotd->nlist_empty);

    while (node->notification.window == 0) {
      pthread_cond_wait(&unotd->notif_open, &unotd->nlist_lock);
    }

    pthread_mutex_unlock(&unotd->nlist_lock);

    pbuf->state = pbuf->buf;
    ProtocolAppend(pbuf, "OK", NULL);
    ProtocolAppendUL(pbuf, UNOT_KEY_NOTIF_ID, not->window);
    goto OK;
  }

  else if (MATCH(pair.key, UNOT_CMD_MODIFY)) {

  }

  else {

    fprintf(stderr, "[unotd:server] ERROR: unknown command '%s'\n", pair.key);
    goto ERROR;
  }

ERROR:
  pbuf->state = pbuf->buf;
  ProtocolAppend(pbuf, "ERROR", NULL);

OK:
  protocol_send(fd, pbuf);
}
