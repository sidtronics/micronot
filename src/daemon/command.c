#include "command.h"
#include "../hf_defs.h"
#include "server.h"
#include "utils.h"
#include <pthread.h>

bool _inject_from_msg(Notification *target, ServerCtx *sctx, hf_message *msg) {

  if (hf_message_has_field_indicator(msg)) {
    indicator_free_str(sctx->display, &target->ind);
    if (!indicator_init_str(sctx->display, &sctx->config, &target->ind,
                            hf_message_get_field_indicator(msg)))
      return false;
  }

  if (hf_message_has_field_text(msg)) {
    free(target->txt);
    target->txt = strdup(hf_message_get_field_text(msg));
  }

  if (hf_message_has_field_text_font(msg)) {

    if (target->custom_txt_font)
      XftFontClose(sctx->display, target->txt_font);

    target->txt_font =
        XftFontOpenName(sctx->display, DefaultScreen(sctx->display),
                        hf_message_get_field_text_font(msg));

    target->custom_txt_font = true;
  }

  if (hf_message_has_field_text_fg(msg)) {

    if (target->custom_txt_color)
      utils_deallocate_color(sctx->display, target->txt_color);

    target->txt_color = utils_allocate_color(
        sctx->display, hf_message_get_field_text_fg(msg), NULL);

    target->custom_txt_color = true;
  }

  if (hf_message_has_field_indicator_fg(msg)) {

    if (target->ind.custom_color)
      utils_deallocate_color(sctx->display, target->ind.color);

    target->ind.color = utils_allocate_color(
        sctx->display, hf_message_get_field_indicator_fg(msg), NULL);

    target->ind.custom_color = true;
  }

  if (hf_message_has_field_timeout(msg))
    target->timeout = hf_message_get_field_timeout(msg);

  return true;
}

static NotificationNode *_create_notification_node(ServerCtx *sctx) {

  NotificationNode *node = calloc(1, sizeof(NotificationNode));
  assert(node && "_create_notification_node: malloc failed");

  /* apply defaults */
  Notification *not = &node->notification;
  not->txt_font = sctx->txt_font;
  not->txt_color = &sctx->txt_color;
  not->ind.color = &sctx->ind_color;
  not->timeout = sctx->config.timeout;
  not->state = UNOT_NEED_INIT;

  return node;
}

static void _delete_notification_node(NotificationNode *node) { free(node); }

static void _handle_command_notify(ServerCtx *sctx, hf_message *msg) {

  if (!hf_message_mask_has_all(msg, UNOT_F_TEXT | UNOT_F_INDICATOR)) {
    hf_message_set_header(msg, UNOT_H_ERROR);
    return;
  }

  NotificationNode *node = _create_notification_node(sctx);

  pthread_mutex_lock(&sctx->nlist_lock);

  if (!_inject_from_msg(&node->notification, sctx, msg)) {
    _delete_notification_node(node);
    hf_message_set_header(msg, UNOT_H_ERROR);
    pthread_mutex_unlock(&sctx->nlist_lock);
    return;
  }

  nlist_append(&sctx->open, node);
  pthread_cond_signal(&sctx->nlist_empty);

  while (node->notification.window == 0) {
    pthread_cond_wait(&sctx->notif_open, &sctx->nlist_lock);
  }

  pthread_mutex_unlock(&sctx->nlist_lock);

  hf_message_set_header(msg, UNOT_H_OK);
  hf_message_set_field_id(msg, node->notification.window);
}

void command_handle(ServerCtx *sctx, hf_message *msg) {

  switch ((hf_message_get_header(msg))) {

  case UNOT_H_NOTIFY:
    _handle_command_notify(sctx, msg);
    break;

  case UNOT_H_MODIFY:
    assert(0 && "not implemented");
    break;

  default:
    assert(0 && "unknown header");
  }
}
