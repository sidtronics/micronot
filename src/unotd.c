#include "unotd.h"
#include "utils.h"
#include <stdint.h>

void unotd_update_notifications(Unotd *unotd) {

  bool unmapped = false;
  NotificationNode *prev = NULL;
  NotificationNode *curr = unotd->open.head;
  while (curr) {

    Notification *nprev = prev ? &prev->notification : NULL;
    Notification *ncurr = &curr->notification;

    if (unmapped && ncurr->needs >= UNOT_NEED_REDRAW) {

      utils_reposition_notification(unotd->display, &unotd->config, nprev,
                                    ncurr);
      notification_move(unotd->display, ncurr);
    }

    switch (ncurr->needs) {

    case UNOT_NEED_INIT:
      unotd_init_notification_resources(unotd, ncurr);
      utils_calculate_notification_layout(unotd->display, &unotd->config,
                                          ncurr);
      utils_reposition_notification(unotd->display, &unotd->config, nprev,
                                    ncurr);
      notification_open(unotd->display, &unotd->config, ncurr);
      pthread_cond_signal(&unotd->notif_open);
      ncurr->needs = UNOT_NEED_UPDATE;
      break;

    case UNOT_NEED_REOPEN:
      utils_reposition_notification(unotd->display, &unotd->config, nprev,
                                    ncurr);
      notification_move(unotd->display, ncurr);
      notification_map(unotd->display, ncurr);
      notification_draw(unotd->display, ncurr);
      ncurr->needs = UNOT_NEED_UPDATE;
      break;

    case UNOT_NEED_REDRAW:
      notification_draw(unotd->display, ncurr);
      ncurr->needs = UNOT_NEED_UPDATE;
      break;

    case UNOT_NEED_UPDATE:
      if (notification_update(unotd->display, ncurr) == 0) {

        unmapped = true;

        switch (ncurr->type) {

        case UNOT_TYPE_MESSAGE:
          unotd_free_notification_resources(unotd, ncurr);
          notification_close(unotd->display, ncurr);
          nlist_remove(&unotd->open, prev);
          break;

        case UNOT_TYPE_SPINNER:
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

void unotd_init_notification_resources(Unotd *unotd, Notification *target) {

  utils_resolve_indicator_font(unotd->display, unotd->config.indicator_size,
                               target);

  if (target->txt_font) {

    char *font_name = (void *)target->txt_font;

    target->txt_font = XftFontOpenName(
        unotd->display, DefaultScreen(unotd->display), font_name);

    ASSERT(target->txt_font &&
           "unotd_allocate_ext_resources: failed to assign txt_font");

    free(font_name);

  } else {
    target->txt_font = unotd->txt_font;
  }

  if (target->txt_color) {

    char *txt_color_str = (void *)target->txt_color;
    target->txt_color =
        utils_allocate_custom_color(unotd->display, txt_color_str);

    free(txt_color_str);

  } else {
    target->txt_color = &unotd->txt_color;
  }

  if (target->ind_color) {

    char *ind_color_str = (void *)target->ind_color;
    target->ind_color =
        utils_allocate_custom_color(unotd->display, ind_color_str);

    free(ind_color_str);

  } else {
    target->ind_color = &unotd->ind_color;
  }
}

void unotd_free_notification_resources(Unotd *unotd, Notification *target) {

  int screen = DefaultScreen(unotd->display);

  if (target->txt_font != unotd->txt_font)
    XftFontClose(unotd->display, target->txt_font);

  if (target->ind_color != &unotd->ind_color) {
    XftColorFree(unotd->display, DefaultVisual(unotd->display, screen),
                 DefaultColormap(unotd->display, screen), target->ind_color);
    free(target->ind_color);
  }

  if (target->txt_color != &unotd->txt_color) {
    XftColorFree(unotd->display, DefaultVisual(unotd->display, screen),
                 DefaultColormap(unotd->display, screen), target->txt_color);
    free(target->txt_color);
  }

  XftFontClose(unotd->display, target->ind_font);
  free(target->text);
}
