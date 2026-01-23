#include "command.h"
#include "parser.h"
#include "utils.h"
#include "server.h"
#include <pthread.h>

void _parse_cmd_ntf(ServerCtx *sctx, Command *cmd, Notification *target) {

  target->txt = strdup(cmd->text);

  if (cmd->custom_indicator) {
    target->ind.str = strdup(cmd->indicator);
    target->ind.custom_string = true;
  } else
    target->ind.str = cmd->indicator;

  indicator_init(sctx->display, &target->ind, sctx->config.indicator_size);

  if (IS_SET(cmd->mask, UNOT_KEY_TEXT_FONT)) {
    target->txt_font = XftFontOpenName(
        sctx->display, DefaultScreen(sctx->display), cmd->text_font);
    target->custom_txt_font = true;
  }

  if (IS_SET(cmd->mask, UNOT_KEY_TEXT_FG)) {
    target->txt_color = utils_allocate_color(sctx->display, cmd->text_fg, NULL);
    target->custom_txt_color = true;
  }

  if (IS_SET(cmd->mask, UNOT_KEY_INDICATOR_FG)) {
    target->ind.color =
        utils_allocate_color(sctx->display, cmd->indicator_fg, NULL);
    target->ind.custom_color = true;
  }

  if (IS_SET(cmd->mask, UNOT_KEY_TIMEOUT))
    target->timeout = cmd->timeout;
}

void command_handle(ServerCtx *sctx, int fd, ProtocolBuffer *pbuf) {

  NotificationNode *node = NULL;

  Command cmd = {0};
  if (!parse_command(pbuf, &sctx->config, &cmd)) {
    goto ERROR;
  }

  if (cmd.kind == UNOT_CMD_NOTIFY) {

    node = calloc(1, sizeof(NotificationNode));
    assert(node && "protocol_handle_command: malloc failed");
    Notification *not = &node->notification;
    not->txt_font = sctx->txt_font;
    not->txt_color = &sctx->txt_color;
    not->ind.color = &sctx->ind_color;
    not->timeout = sctx->config.timeout;
    not->state = UNOT_NEED_INIT;

    pthread_mutex_lock(&sctx->nlist_lock);

    _parse_cmd_ntf(sctx, &cmd, not);

    nlist_append(&sctx->open, node);
    pthread_cond_signal(&sctx->nlist_empty);

    while (node->notification.window == 0) {
      pthread_cond_wait(&sctx->notif_open, &sctx->nlist_lock);
    }

    pthread_mutex_unlock(&sctx->nlist_lock);

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
