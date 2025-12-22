#include "command.h"
// #include "../protocol.h"
#include "parser.h"
#include "utils.h"
#include <pthread.h>

bool _parse_cmd_ntf(Unotd *unotd, Command *cmd, Notification *target) {

  const char *text = NULL;
  const char *indicator = NULL;
  bool name = false;
  if (cmd->mask & UNOT_KEY_TEXT)
    text = cmd->text;

  if (cmd->mask & UNOT_KEY_INDICATOR)
    indicator = cmd->indicator;

  if (cmd->mask & UNOT_KEY_INDICATOR_NAME) {

    for (size_t i = 0; i < unotd->config.indicators_count; i++) {
      char *indicator_name = unotd->config.indicators[i];
      if (MATCH(cmd->indicator_name, indicator_name)) {
        indicator = indicator_name + strlen(indicator_name) + 1;
        name = true;
        break;
      }
    }

    if (!name)
      fprintf(stderr, "[unotd:server] WARN: indicator of name '%s' not found\n",
              cmd->indicator_name);
  }

  if (cmd->mask & UNOT_KEY_TEXT_FONT)
    target->txt_font = XftFontOpenName(
        unotd->display, DefaultScreen(unotd->display), cmd->text_font);

  if (cmd->mask & UNOT_KEY_TEXT_FG)
    target->txt_color =
        utils_allocate_color(unotd->display, cmd->text_fg, NULL);

  if (cmd->mask & UNOT_KEY_INDICATOR_FG)
    target->ind.color =
        utils_allocate_color(unotd->display, cmd->indicator_fg, NULL);

  if (cmd->mask & UNOT_KEY_TIMEOUT)
    target->timeout = cmd->timeout;

  if (!text) {
    // fprintf(stderr, "[unotd:server] ERROR: missing key: '%s'\n",
    // UNOT_KEY_TEXT);
    return false;
  }

  if (!indicator) {
    // fprintf(stderr, "[unotd:server] ERROR: missing key: '%s' or '%s'\n",
    //         UNOT_KEY_INDICATOR, UNOT_KEY_INDICATOR_NAME);
    return false;
  }

  if (name)
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
    indicator = memcpy(target->txt + txt_len, indicator, ind_len);
  }

  indicator_init(unotd->display, &target->ind, indicator,
                 unotd->config.indicator_size);

  return true;
}

// const char *key;
// const char *val;
// while (protocol_parse_field(pbuf, &key, &val)) {
//
//   if (!key)
//     return false;
//
//   if (MATCH(key, UNOT_KEY_TEXT))
//     text = val;
//
//   else if (MATCH(key, UNOT_KEY_INDICATOR))
//     indicator = val;
//
//   else if (MATCH(key, UNOT_KEY_INDICATOR_NAME)) {
//     for (size_t i = 0; i < unotd->config.indicators_count; i++) {
//       char *indicator_name = unotd->config.indicators[i];
//       if (MATCH(val, indicator_name)) {
//         indicator = indicator_name + strlen(indicator_name) + 1;
//         name = true;
//         break;
//       }
//     }
//
//     if (!name)
//       fprintf(stderr, "[unotd:server] WARN: indicator of name '%s' not found\n",
//               val);
//   }
//
//   else if (MATCH(key, UNOT_KEY_TEXT_FONT))
//     target->txt_font =
//         XftFontOpenName(unotd->display, DefaultScreen(unotd->display), val);
//
//   else if (MATCH(key, UNOT_KEY_TEXT_FG))
//     target->txt_color = utils_allocate_color_s(unotd->display, val, NULL);
//
//   else if (MATCH(key, UNOT_KEY_INDICATOR_FG))
//     target->ind.color = utils_allocate_color_s(unotd->display, val, NULL);
//
//   else if (MATCH(key, UNOT_KEY_TIMEOUT)) {
//     if (!utils_parse_ul(val, &target->timeout, 10)) {
//       fprintf(stderr, "[unotd:server] ERROR: invalid timeout value: '%s'\n",
//               val);
//       return false;
//     }
//   }
//
//   else {
//     fprintf(stderr, "[unotd:server] ERROR: unknown key: '%s'\n", key);
//     return false;
//   }
// }
//
// if (!text) {
//   fprintf(stderr, "[unotd:server] ERROR: missing key: '%s'\n", UNOT_KEY_TEXT);
//   return false;
// }
//
// if (!indicator) {
//   fprintf(stderr, "[unotd:server] ERROR: missing key: '%s' or '%s'\n",
//           UNOT_KEY_INDICATOR, UNOT_KEY_INDICATOR_NAME);
//   return false;
// }
//
// if (name)
//   target->txt = strdup(text);
//
// else {
//
//   if (!indicator_validate(indicator)) {
//     fprintf(stderr, "[unotd:server] ERROR: malformed indicator string\n");
//     return false;
//   }
//
//   size_t txt_len = strlen(text) + 1;
//   size_t ind_len = strlen(indicator) + 1;
//   target->txt = malloc(txt_len + ind_len);
//   memcpy(target->txt, text, txt_len);
//   indicator = memcpy(target->txt + txt_len, indicator, ind_len);
// }
//
// indicator_init(unotd->display, &target->ind, indicator,
//                unotd->config.indicator_size);
//
// return true;
// }

void command_handle(Unotd *unotd, int fd, ProtocolBuffer *pbuf) {

  NotificationNode *node = NULL;

  // const char *cmd;
  // protocol_parse_header(pbuf, &cmd);
  // if (*cmd == 0) {
  //   fprintf(stderr, "[unotd:server] ERROR: missing command\n");
  //   goto ERROR;
  // }

  Command cmd = {0};
  if (!parse_command(pbuf, &cmd)) {
    goto ERROR;
  }

  if (cmd.kind == UNOT_CMD_NOTIFY) {

    node = calloc(1, sizeof(NotificationNode));
    assert(node && "protocol_handle_command: malloc failed");
    Notification *not = &node->notification;
    not->txt_font = unotd->txt_font;
    not->txt_color = &unotd->txt_color;
    not->ind.color = &unotd->ind_color;
    not->timeout = unotd->config.timeout;
    not->state = UNOT_NEED_INIT;

    pthread_mutex_lock(&unotd->nlist_lock);

    if (!_parse_cmd_ntf(unotd, &cmd, not)) {
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
    protocol_append_header(pbuf, "OK");
    protocol_append(pbuf, "nid", not->window);
    goto OK;
  }

  else if (cmd.kind == UNOT_CMD_MODIFY) {
  }

ERROR:
  pbuf->state = pbuf->buf;
  protocol_append_header(pbuf, "ERROR");

OK:
  protocol_send(fd, pbuf);
}
