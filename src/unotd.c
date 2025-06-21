#include "unotd.h"

XftFont *unotd_resolve_indicator_font(Unotd *unotd, const char *indicator) {

  FcCharSet *charset = FcCharSetCreate();
  FcPattern *pattern = FcPatternCreate();
  FcPattern *matched_pattern;

  const char *p = indicator;

  while (*p) {

    wchar_t wc;
    int n = mbtowc(&wc, p, MB_CUR_MAX);
    assert(n > 0 && "mbtowc failed");

    FcCharSetAddChar(charset, (FcChar32)wc);
    p += n;
    if (*p == 0 || *p == 30)
      p++;
  }

  FcPatternAddCharSet(pattern, FC_CHARSET, charset);
  FcPatternAddDouble(pattern, FC_SIZE, unotd->config.indicator_size);
  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);

  FcResult result;
  matched_pattern = FcFontMatch(NULL, pattern, &result);
  assert(result == FcResultMatch);

  XftFont *font = XftFontOpenPattern(unotd->display, matched_pattern);
  assert(font && "cant open Xftfont");

  FcCharSetDestroy(charset);
  FcPatternDestroy(pattern);

  return font;
}

void unotd_calc_notification_layout(Unotd *unotd, Notification *notification) {

  const char *p = notification->indicator;
  XGlyphInfo ind_extents = {0};
  while (*p) {

    const char *end = strchrnul(p, 30);
    XGlyphInfo temp;
    XftTextExtentsUtf8(unotd->display, notification->ind_font, (FcChar8 *)p,
                       (size_t)(end - p), &temp);

    if (ind_extents.height < temp.height)
      ind_extents.height = temp.height;
    if (ind_extents.width < temp.width)
      ind_extents.width = temp.width;
    if (ind_extents.y < temp.y)
      ind_extents.y = temp.y;

    p = end + 1;
  }

  XGlyphInfo extents;
  XftTextExtentsUtf8(unotd->display, notification->msg_font,
                     (FcChar8 *)notification->message,
                     strlen(notification->message), &extents);

  Config config = unotd->config;
  uint8_t y_padding = config.y_padding;
  uint8_t spacing = config.spacing;
  uint8_t bor_w = config.border_thickness;
  uint8_t x_padding = config.x_padding;

  uint8_t msg_w = extents.width;
  uint8_t ind_w = ind_extents.width;

  uint8_t max_h = MAX(extents.height, ind_extents.height);

  notification->win_w = ind_w + spacing + msg_w + x_padding * 2;
  notification->win_h = max_h + y_padding * 2;

  notification->ind_x = x_padding;
  notification->msg_x = x_padding + ind_w + spacing;
  notification->ind_y =
      ind_extents.y + (notification->win_h - ind_extents.height) / 2;
  notification->msg_y = extents.y + (notification->win_h - extents.height) / 2;
}

void unotd_reposition_notification(Unotd *unotd, NotificationNode *previous,
                                   NotificationNode *current) {

  Screen *screen = DefaultScreenOfDisplay(unotd->display);

  Notification *prev = &previous->notification;
  Notification *target = &current->notification;

  uint8_t bor_w = unotd->config.border_thickness;
  uint8_t gap_size = unotd->config.gap_size;

  switch (unotd->origin) {

  case UNOT_ORIGIN_TOP_RIGHT:
    target->win_x = screen->width - target->win_w - 2 * bor_w;
    target->win_y =
        previous
            ? (target->win_y = prev->win_y + prev->win_h + 2 * bor_w + gap_size)
            : 0;

    break;

  case UNOT_ORIGIN_TOP_LEFT:
    target->win_x = 0;
    target->win_y =
        previous
            ? (target->win_y = prev->win_y + prev->win_h + 2 * bor_w + gap_size)
            : 0;
    break;

  case UNOT_ORIGIN_BOTTOM_RIGHT:
    target->win_x = screen->width - target->win_w - 2 * bor_w;
    target->win_y =
        previous ? (target->win_y =
                        prev->win_y - gap_size - target->win_h - 2 * bor_w)
                 : (target->win_y = screen->height - target->win_h - 2 * bor_w);

    break;

  case UNOT_ORIGIN_BOTTOM_LEFT:
    target->win_x = 0;
    target->win_y =
        previous ? (target->win_y =
                        prev->win_y - gap_size - target->win_h - 2 * bor_w)
                 : (target->win_y = screen->height - target->win_h - 2 * bor_w);
    break;
  }
}

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
      unotd_handle_unmapped_notification(unotd, e.xunmap.window);
      break;
    }
  }
}

void unotd_update_notifications(Unotd *unotd) {

  if (unotd->head_open == NULL)
    return;

  NotificationNode *previous = NULL;
  NotificationNode *current = unotd->head_open;
  while (current) {

    if (current->notification.window == 0) {
      unotd_allocate_ext_resources(unotd, &current->notification);
      unotd_calc_notification_layout(unotd, &current->notification);
      unotd_reposition_notification(unotd, previous, current);
      notification_open(unotd->display, &unotd->config, &current->notification);
    } else {
      notification_update(unotd->display, &current->notification);
    }
    previous = current;
    current = current->next;
  }
  XSync(unotd->display, False);
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

  if (unotd->head_open != NULL) {

    NotificationNode *current = (previous ? previous->next : unotd->head_open);

    while (current) {

      unotd_reposition_notification(unotd, previous, current);
      XMoveWindow(unotd->display, current->notification.window,
                  current->notification.win_x, current->notification.win_y);
      previous = current;
      current = current->next;
    }
  }
}

static XftColor *_allocate_custom_color(Display *dpy, const char *color_str) {

  unsigned long color = strtoul(color_str, NULL, 16);
  // assert(color && "_allocate_custom_color: failed to parse color string");

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
    notification->msg_color =
        _allocate_custom_color(unotd->display, msg_color_str);

  } else {
    notification->msg_color = &unotd->msg_color;
  }

  if (notification->ind_color) {

    const char *ind_color_str = (const char *)notification->ind_color;
    notification->ind_color =
        _allocate_custom_color(unotd->display, ind_color_str);

  } else {
    notification->ind_color = &unotd->ind_color;
  }

  if (notification->frame)
    free((void *)notification->frame);
}

void unotd_free_ext_resources(Unotd *unotd, Notification *notification) {

  int screen = DefaultScreen(unotd->display);

  if (notification->msg_font != unotd->msg_font)
    XftFontClose(unotd->display, notification->msg_font);

  if (notification->ind_color != &unotd->ind_color) {
    XftColorFree(unotd->display, DefaultVisual(unotd->display, screen),
                 DefaultColormap(unotd->display, screen),
                 notification->ind_color);
    free(notification->ind_color);
  }

  if (notification->msg_color != &unotd->msg_color) {
    XftColorFree(unotd->display, DefaultVisual(unotd->display, screen),
                 DefaultColormap(unotd->display, screen),
                 notification->msg_color);
    free(notification->msg_color);
  }

  XftFontClose(unotd->display, notification->ind_font);
  free((void *)notification->message);
}
