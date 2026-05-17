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

  if (hf_message_has_field_persistent(msg))
    target->is_persistent = hf_message_get_field_persistent(msg);

  return true;
}

static NotificationNode *_create_notification_node(ServerCtx *sctx,
                                                   ClientID cid) {

  NotificationNode *node = calloc(1, sizeof(NotificationNode));
  assert(node && "_create_notification_node: malloc failed");

  Notification *not = &node->notification;
  not->cid = cid;

  /* apply defaults */
  not->txt_font = sctx->txt_font;
  not->txt_color = &sctx->txt_color;
  not->ind.color = &sctx->ind_color;
  not->timeout = sctx->config.timeout;
  not->state = UNOT_NEED_INIT;

  return node;
}

static void _delete_notification_node(NotificationNode *node) { free(node); }

static void _handle_command_notify(ServerCtx *sctx, ClientID cid,
                                   hf_message *msg) {

  if (!hf_message_mask_has_all(msg, UNOT_F_TEXT)) {
    hf_message_set_header(msg, UNOT_H_ERROR);
    return;
  }

  NotificationNode *node = _create_notification_node(sctx, cid);

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

void _handle_command_modify(ServerCtx *sctx, ClientID cid, hf_message *msg) {

  if (!hf_message_has_field_id(msg)) {
    hf_message_set_header(msg, UNOT_H_ERROR);
    return;
  }

  NotificationID id = hf_message_get_field_id(msg);
  NotificationNode *prev, *node;

  pthread_mutex_lock(&sctx->nlist_lock);

  if ((node = nlist_find(&sctx->open, &prev, id)) &&
      node->notification.cid == cid) {

    // Found in open list
    Notification *not = &node->notification;

    if (!_inject_from_msg(not, sctx, msg)) {
      hf_message_set_header(msg, UNOT_H_ERROR);
      pthread_mutex_unlock(&sctx->nlist_lock);
      return;
    }

    if (hf_message_has_field_timeout(msg))
      clock_gettime(CLOCK_MONOTONIC, &not->start_time);

    not->state = UNOT_NEED_REDRAW;
  }

  else if ((node = nlist_find(&sctx->wait, &prev, id)) &&
           node->notification.cid == cid) {

    // Found in wait list
    Notification *not = &node->notification;

    if (!_inject_from_msg(not, sctx, msg)) {
      hf_message_set_header(msg, UNOT_H_ERROR);
      pthread_mutex_unlock(&sctx->nlist_lock);
      return;
    }

    clock_gettime(CLOCK_MONOTONIC, &not->start_time);

    not->state = UNOT_NEED_REOPEN;

    nlist_unlink(&sctx->wait, prev);
    nlist_append(&sctx->open, node);
  }

  else {
    hf_message_set_header(msg, UNOT_H_ERROR);
    pthread_mutex_unlock(&sctx->nlist_lock);
    return;
  }

  pthread_mutex_unlock(&sctx->nlist_lock);

  hf_message_set_header(msg, UNOT_H_OK);
}

void _handle_command_debug(ServerCtx *sctx, hf_message *msg) {

  int open_count = 0;
  int wait_count = 0;

  pthread_mutex_lock(&sctx->nlist_lock);

  const char *row_header = "  %-6s | %-30s | %-10s | %s\n";
  const char *row_values = "  %-6s | %-30s | %-10lu | %s\n";
  const char *sep =
      "  -------+--------------------------------+------------+-----------\n";

  fprintf(stderr, "[unotd:server]: DEBUG:\n");

  if (sctx->wait.head || sctx->open.head) {

    fprintf(stderr, "\n");
    fprintf(stderr, "%s", sep);
    fprintf(stderr, row_header, "list", "text", "client", "persistent");
    fprintf(stderr, "%s", sep);

    NotificationNode *curr = sctx->open.head;
    while (curr != NULL) {
      open_count++;
      Notification *n = &curr->notification;
      fprintf(stderr, row_values, "[OPEN]", n->txt, n->cid,
              n->is_persistent ? "*" : " ");
      curr = curr->next;
    }

    curr = sctx->wait.head;

    if (curr)
      fprintf(stderr, "%s", sep);

    while (curr != NULL) {
      wait_count++;
      Notification *n = &curr->notification;
      fprintf(stderr, row_values, "[WAIT]", n->txt, n->cid,
              n->is_persistent ? "*" : " ");
      curr = curr->next;
    }

    fprintf(stderr, "%s", sep);
  }

  fprintf(stderr, "\n  Open count: %d\n  Wait count: %d\n\n", open_count,
          wait_count);

  pthread_mutex_unlock(&sctx->nlist_lock);

  hf_message_set_header(msg, UNOT_H_OK);
}

void command_handle(ServerCtx *sctx, ClientID cid, hf_message *msg) {

  switch ((hf_message_get_header(msg))) {

  case UNOT_H_NOTIFY:
    _handle_command_notify(sctx, cid, msg);
    break;

  case UNOT_H_MODIFY:
    _handle_command_modify(sctx, cid, msg);
    break;

  case UNOT_H_DEBUG:
    _handle_command_debug(sctx, msg);
    break;

  default:
    assert(0 && "unknown header");
  }
}
