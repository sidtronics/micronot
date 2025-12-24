#include "command.h"
#include "parser.h"
#include "utils.h"
#include <pthread.h>

void _parse_cmd_ntf(Unotd *unotd, Command *cmd, Notification *target) {

  target->txt = strdup(cmd->text);

  if (cmd->custom_indicator) {
    target->ind.str = strdup(cmd->indicator);
    target->ind.custom_string = true;
  } else
    target->ind.str = cmd->indicator;

  indicator_init(unotd->display, &target->ind, unotd->config.indicator_size);

  if (IS_SET(cmd->mask, UNOT_KEY_TEXT_FONT)) {
    target->txt_font = XftFontOpenName(
        unotd->display, DefaultScreen(unotd->display), cmd->text_font);
    target->custom_txt_font = true;
  }

  if (IS_SET(cmd->mask, UNOT_KEY_TEXT_FG)) {
    target->txt_color =
        utils_allocate_color(unotd->display, cmd->text_fg, NULL);
    target->custom_txt_color = true;
  }

  if (IS_SET(cmd->mask, UNOT_KEY_INDICATOR_FG)) {
    target->ind.color =
        utils_allocate_color(unotd->display, cmd->indicator_fg, NULL);
    target->ind.custom_color = true;
  }

  if (IS_SET(cmd->mask, UNOT_KEY_TIMEOUT))
    target->timeout = cmd->timeout;
}

void command_handle(Unotd *unotd, int fd, ProtocolBuffer *pbuf) {

  NotificationNode *node = NULL;

  Command cmd = {0};
  if (!parse_command(pbuf, &unotd->config, &cmd)) {
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

    _parse_cmd_ntf(unotd, &cmd, not);

    nlist_append(&unotd->open, node);
    pthread_cond_signal(&unotd->nlist_empty);

    while (node->notification.window == 0) {
      pthread_cond_wait(&unotd->notif_open, &unotd->nlist_lock);
    }

    pthread_mutex_unlock(&unotd->nlist_lock);

    protocol_append_header(pbuf, "OK");
    protocol_append(pbuf, "nid", not->window);
    goto OK;
  }

  else if (cmd.kind == UNOT_CMD_MODIFY) {
    assert(0 && "TODO");
  }

ERROR:
  pbuf->state = pbuf->buf;
  protocol_append_header(pbuf, "ERR");

OK:
  protocol_send(fd, pbuf);
}
