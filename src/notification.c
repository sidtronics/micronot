#include "notification.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define GET_WIN_COORD(scr, win, bor) (scr) - (win) - 2 * (bor)

Notification *OpenNotification(Display *dpy, Config *config,
                                       XftFont *font, const char *message,
                                       NotificationType type,
                                       void *indicator, uint8_t timeout) {

  Notification *res = (Notification *)malloc(sizeof(Notification));

  res->messsage = message;
  res->font = font;
  res->timeout = timeout;
  res->type = type;

  XGlyphInfo indicator_extents;
  switch (type) {
  case UNOT_MESSAGE:
    res->icon = (Icon *)indicator;
    indicator_extents = ((Icon *)indicator)->extents;
    break;
  case UNOT_SPINNER:
    res->spinner = (Spinner *)indicator;
    indicator_extents = ((Spinner *)indicator)->extents;
    break;
  }

  XGlyphInfo extents;
  XftTextExtentsUtf8(dpy, font, (FcChar8 *)message, strlen(message), &extents);

  uint8_t x_padding = config->x_padding;
  uint8_t y_padding = config->y_padding;
  uint8_t spacing = config->spacing;
  uint8_t bor_w = config->border_thickness;

  uint8_t t_w = extents.width;
  uint8_t i_w = indicator_extents.width;

  uint8_t h = MAX(extents.height, indicator_extents.height);

  uint8_t win_w = i_w + spacing + t_w + x_padding * 2;
  uint8_t win_h = h + y_padding * 2;

  res->i_x = x_padding;
  res->t_x = x_padding + i_w + spacing;

  if (indicator_extents.height >= extents.height) {
    res->i_y = y_padding + indicator_extents.y;
    res->t_y =
        y_padding + (indicator_extents.height - extents.height) / 2 + extents.y;
  } else {
    res->t_y = y_padding + extents.y;
    res->i_y = y_padding + (extents.height - indicator_extents.height) / 2 +
               indicator_extents.y;
  }

  Screen *screen = XDefaultScreenOfDisplay(dpy);
  u_int32_t scr_nbr = XScreenNumberOfScreen(screen);

  u_int32_t mask =
      CWBackPixel | CWBorderPixel | CWOverrideRedirect | CWEventMask;
  XSetWindowAttributes attrs;
  attrs.background_pixel = config->background_color;
  attrs.border_pixel = config->border_color;
  attrs.event_mask = ExposureMask | ButtonPressMask;
  attrs.override_redirect = 1;

  res->window = XCreateWindow(dpy, XRootWindowOfScreen(screen),
                              GET_WIN_COORD(screen->width, win_w, bor_w),
                              GET_WIN_COORD(screen->height, win_h, bor_w),
                              win_w, win_h, bor_w, CopyFromParent, InputOutput,
                              CopyFromParent, mask, &attrs);

  res->draw = XftDrawCreate(dpy, res->window, DefaultVisual(dpy, scr_nbr),
                            DefaultColormap(dpy, scr_nbr));

  XRenderColor xrcolor = {
      .red = ((config->foreground_color >> 16) & 0xff) * 257,
      .green = ((config->foreground_color >> 8) & 0xff) * 257,
      .blue = (config->foreground_color & 0xff) * 257,
      .alpha = 0xffff};
  XftColorAllocValue(dpy, DefaultVisual(dpy, scr_nbr),
                     DefaultColormap(dpy, scr_nbr), &xrcolor, &res->color);

  XSelectInput(dpy, res->window, ExposureMask | ButtonPressMask);

  XMapWindow(dpy, res->window);
  XSync(dpy, False);

  XftFont *indicator_font;
  const char *indicator_str;
  switch (type) {
  case UNOT_MESSAGE:
    indicator_font = ((Icon *)indicator)->font;
    indicator_str = ((Icon *)indicator)->icon;
    break;

  case UNOT_SPINNER:
    indicator_font = ((Spinner *)indicator)->font;
    indicator_str = ((Spinner *)indicator)->frames[0];
    res->current_frame = 0;
    break;
  }

  XftDrawStringUtf8(res->draw, &res->color, indicator_font, res->i_x, res->i_y,
                    (FcChar8 *)indicator_str, strlen(indicator_str));

  XftDrawStringUtf8(res->draw, &res->color, res->font, res->t_x, res->t_y,
                    (FcChar8 *)res->messsage, strlen(res->messsage));

  clock_gettime(CLOCK_MONOTONIC, &res->last_updated);

  XFlush(dpy);

  return res;
}

void UpdateNotification(Display *dpy, Notification *notification) {

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  long elapsed = (now.tv_sec - notification->last_updated.tv_sec) * 1000 +
                 (now.tv_nsec - notification->last_updated.tv_nsec) / 1000000;

  if (notification->type == UNOT_MESSAGE) {

    if (elapsed >= notification->timeout * 1000) {
      XUnmapWindow(dpy, notification->window);
    }

  }

  else if (notification->type == UNOT_SPINNER) {

    if (elapsed >= 200) {

      notification->current_frame =
          (notification->current_frame + 1) % notification->spinner->count;

      XClearArea(dpy, notification->window, 0, 0, notification->t_x, 0, False);

      const char *frame =
          notification->spinner->frames[notification->current_frame];
      XftDrawStringUtf8(notification->draw, &notification->color,
                        notification->spinner->font, notification->i_x,
                        notification->i_y, (FcChar8 *)frame, strlen(frame));

      XFlush(dpy);
      notification->last_updated = now;
    }
  }
}
