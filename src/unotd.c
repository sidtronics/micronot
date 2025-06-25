#include "unotd.h"
#include "utils.h"
#include <stdint.h>

void unotd_handle_events(Unotd *unotd) {

  XEvent e;
  while (XPending(unotd->display)) {

    XNextEvent(unotd->display, &e);
    switch (e.type) {

    case Expose:
      break;
    case ButtonPress:
      if (e.xbutton.button == Button1) {
        XUnmapWindow(unotd->display, e.xbutton.window);
        XSync(unotd->display, False);
      }
      break;
    case UnmapNotify:
      unotd_handle_unmap(unotd, e.xunmap.window);
      break;
    }
  }
}

void unotd_handle_unmap(Unotd *unotd, Window window) {

  NotificationNode *previous = NULL;
  NotificationNode *unmapped = notification_list_find(
      &unotd->open, &previous, unotd_match_window, (void *)(uintptr_t)window);

  ASSERT(unmapped && "unotd_handle_unmapped_notification: node not found");

  switch (unmapped->notification.type) {

  case UNOT_MESSAGE:
    unotd_free_notification_resources(unotd, &unmapped->notification);
    notification_close(unotd->display, &unmapped->notification);
    notification_list_remove(&unotd->open, previous);
    break;

  case UNOT_SPINNER:
    notification_list_unlink(&unotd->open, previous);
    notification_list_append(&unotd->open, unmapped);
    break;
  }

  if (!notification_list_is_empty(&unotd->open)) {

    notification_list_foreach(&unotd->open, previous, unotd_reposition_visitor,
                              unotd);
  }
}

bool unotd_match_window(Notification *node, void *data) {

  Window target = (Window)(uintptr_t)data;
  return node->window == target;
}

void unotd_update_visitor(Notification *prev, Notification *curr, void *data) {

  Unotd *unotd = (Unotd *)data;

  if (curr->window == 0) {
    unotd_init_notification_resources(unotd, curr);
    utils_calculate_notification_layout(unotd->display, &unotd->config, curr);
    utils_reposition_notification(unotd->display, &unotd->config, prev, curr);
    notification_open(unotd->display, &unotd->config, curr);
  } else {
    notification_update(unotd->display, curr);
  }
}

void unotd_reposition_visitor(Notification *prev, Notification *curr,
                              void *data) {

  Unotd *unotd = (Unotd *)data;
  utils_reposition_notification(unotd->display, &unotd->config, prev, curr);
  XMoveWindow(unotd->display, curr->window, curr->win_x, curr->win_y);
}

void unotd_init_notification_resources(Unotd *unotd, Notification *target) {

  utils_resolve_indicator_font(unotd->display, unotd->config.indicator_size,
                               target);

  if (target->msg_font) {

    const char *font_name = (const char *)target->msg_font;
    target->msg_font = XftFontOpenName(
        unotd->display, DefaultScreen(unotd->display), font_name);

    ASSERT(target->msg_font &&
           "unotd_allocate_ext_resources: failed to assign msg_font");
  } else {
    target->msg_font = unotd->msg_font;
  }

  if (target->msg_color) {

    const char *msg_color_str = (const char *)target->msg_color;
    target->msg_color =
        utils_allocate_custom_color(unotd->display, msg_color_str);

  } else {
    target->msg_color = &unotd->msg_color;
  }

  if (target->ind_color) {

    const char *ind_color_str = (const char *)target->ind_color;
    target->ind_color =
        utils_allocate_custom_color(unotd->display, ind_color_str);

  } else {
    target->ind_color = &unotd->ind_color;
  }

  if (target->frame)
    free((void *)target->frame);
}

void unotd_free_notification_resources(Unotd *unotd, Notification *target) {

  int screen = DefaultScreen(unotd->display);

  if (target->msg_font != unotd->msg_font)
    XftFontClose(unotd->display, target->msg_font);

  if (target->ind_color != &unotd->ind_color) {
    XftColorFree(unotd->display, DefaultVisual(unotd->display, screen),
                 DefaultColormap(unotd->display, screen), target->ind_color);
    free(target->ind_color);
  }

  if (target->msg_color != &unotd->msg_color) {
    XftColorFree(unotd->display, DefaultVisual(unotd->display, screen),
                 DefaultColormap(unotd->display, screen), target->msg_color);
    free(target->msg_color);
  }

  XftFontClose(unotd->display, target->ind_font);
  free((void *)target->message);
}
