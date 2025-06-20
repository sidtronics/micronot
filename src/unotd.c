#include "unotd.h"

void unotd_handle_events(Unotd *unotd) {

  XEvent e;
  while (XPending(unotd->display)) {

    XNextEvent(unotd->display, &e);
    switch (e.type) {

    case Expose:
      break;
    case ButtonPress:
      if (e.xbutton.button == Button1)
        XUnmapWindow(unotd->display, e.xbutton.window);
      break;
    case UnmapNotify:
      unotd_handle_unmapped_notification(unotd, e.xunmap.window);
      break;
    }
  }
}

void unotd_update_notifications(Unotd *unotd) {

  if (unotd->head_open == NULL)
    return;

  NotificationNode *current = unotd->head_open;
  while (current) {

    notification_update(unotd->display, &current->notification);
    current = current->next;
  }
}

void unotd_handle_unmapped_notification(Unotd *unotd, Window window) {

  NotificationNode *previous = NULL;
  NotificationNode *unmapped =
      notification_list_find_by_window(unotd->head_open, &previous, window);

  if (!unmapped)
    assert(0 && "unotd_handle_unmapped_notification: node not found");

  switch (unmapped->notification.type) {

  case UNOT_MESSAGE:
    unotd_free_ext_resources(unotd, &unmapped->notification);
    notification_close(unotd->display, &unmapped->notification);
    notification_list_remove_next(&unotd->head_open, previous);
    break;

  case UNOT_SPINNER:
    notification_list_unlink_next(&unotd->head_open, previous);
    notification_list_append(&unotd->head_waiting, unmapped);
    break;
  }
}

static XftColor *_allocate_custom_color(Display *dpy, const char* color_str) {

  unsigned long color = strtoul(color_str, NULL, 16);
  //assert(color && "_allocate_custom_color: failed to parse color string");

  XRenderColor xrcolor = {.red = ((color >> 16) & 0xff) * 257,
                          .green = ((color >> 8) & 0xff) * 257,
                          .blue = (color & 0xff) * 257,
                          .alpha = 0xffff};

  int scr_nbr = DefaultScreen(dpy);

  XftColor *res = malloc(sizeof(XftColor));

  XftColorAllocValue(dpy, DefaultVisual(dpy, scr_nbr),
                     DefaultColormap(dpy, scr_nbr), &xrcolor, res);

  return res;
}

void unotd_allocate_ext_resources(Unotd *unotd, Notification *notification) {

  if (notification->msg_font) {

    const char *font_name = (const char *)notification->msg_font;
    notification->msg_font = XftFontOpenName(
        unotd->display, DefaultScreen(unotd->display), font_name);

    assert(notification->msg_font &&
           "unotd_allocate_ext_resources: failed to assign msg_font");
  } else {
    notification->msg_font = unotd->msg_font;
  }

  if (notification->msg_color) {

    const char *msg_color_str = (const char *)notification->msg_color;
    notification->msg_color = _allocate_custom_color(unotd->display, msg_color_str);

  } else {
    notification->msg_color = &unotd->msg_color;
  }

  if (notification->ind_color) {

    const char *ind_color_str = (const char *)notification->ind_color;
    notification->ind_color = _allocate_custom_color(unotd->display, ind_color_str);

  } else {
    notification->ind_color = &unotd->ind_color;
  }

  if (notification->frame)
    free((void *)notification->frame);
}

void unotd_free_ext_resources(Unotd *unotd, Notification *notification) {

  if (notification->msg_font != unotd->msg_font)
    free(notification->msg_font);

  if (notification->ind_color != &unotd->ind_color)
    free(notification->ind_color);

  if (notification->msg_color != &unotd->msg_color)
    free(notification->msg_color);

  free(notification->ind_font);
  free((void *)notification->message);
}
