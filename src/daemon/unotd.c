#include "unotd.h"
#include "utils.h"
#include <pthread.h>
#include <stdint.h>

static void _free_notification_resources(Unotd *unotd, Notification *target) {

  int screen = DefaultScreen(unotd->display);

  if (target->txt_font != unotd->txt_font)
    XftFontClose(unotd->display, target->txt_font);

  if (target->ind.color != &unotd->ind_color) {
    XftColorFree(unotd->display, DefaultVisual(unotd->display, screen),
                 DefaultColormap(unotd->display, screen), target->ind.color);
    free(target->ind.color);
  }

  if (target->txt_color != &unotd->txt_color) {
    XftColorFree(unotd->display, DefaultVisual(unotd->display, screen),
                 DefaultColormap(unotd->display, screen), target->txt_color);
    free(target->txt_color);
  }

  indicator_free_font(unotd->display, &target->ind);
  free(target->txt);
}

void unotd_update_notifications(Unotd *unotd) {

  bool needs_reposition = false;
  NotificationNode *prev = NULL;
  NotificationNode *curr = unotd->open.head;
  while (curr) {

    Notification *nprev = prev ? &prev->notification : NULL;
    Notification *ncurr = &curr->notification;

    switch (ncurr->state) {

    case UNOT_NEED_INIT:
      utils_calculate_notification_layout(unotd->display, &unotd->config,
                                          ncurr);
      utils_reposition_notification(unotd->display, &unotd->config, nprev,
                                    ncurr);
      notification_open(unotd->display, &unotd->config, ncurr);
      pthread_cond_signal(&unotd->notif_open);
      ncurr->state = UNOT_NEED_UPDATE;
      break;

    case UNOT_NEED_REDRAW:
      needs_reposition = true;
    case UNOT_NEED_REOPEN:
      utils_reposition_notification(unotd->display, &unotd->config, nprev,
                                    ncurr);
      notification_move(unotd->display, ncurr);
      notification_map(unotd->display, ncurr);
      notification_draw(unotd->display, ncurr);
      ncurr->state = UNOT_NEED_UPDATE;
      break;

    case UNOT_NEED_UPDATE:

      if (needs_reposition) {

        utils_reposition_notification(unotd->display, &unotd->config, nprev,
                                      ncurr);
        notification_move(unotd->display, ncurr);
      }

      if (notification_update(unotd->display, ncurr) == 0) {

        needs_reposition = true;

        switch (ncurr->ind.type) {

        case INDICATOR_TYPE_ICON:
          _free_notification_resources(unotd, ncurr);
          notification_close(unotd->display, ncurr);
          nlist_remove(&unotd->open, prev);
          break;

        case INDICATOR_TYPE_SPINNER:
          nlist_unlink(&unotd->open, prev);
          nlist_append(&unotd->wait, curr);
          break;
        }

        curr = prev ? prev->next : unotd->open.head;
        continue;
      }
    }

    prev = curr;
    curr = curr->next;
  }
}
