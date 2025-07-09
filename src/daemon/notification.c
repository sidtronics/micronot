#include "notification.h"
#include "utils.h"

static Window _create_notification_window(Display *dpy, u_int16_t win_x,
                                          u_int16_t win_y, u_int16_t win_w,
                                          u_int16_t win_h, u_int16_t bor_w,
                                          unsigned long bg_color,
                                          unsigned long bor_color) {

  Screen *screen = DefaultScreenOfDisplay(dpy);

  u_int32_t mask =
      CWBackPixel | CWBorderPixel | CWOverrideRedirect | CWEventMask;
  XSetWindowAttributes attrs;
  attrs.background_pixel = bg_color;
  attrs.border_pixel = bor_color;
  attrs.event_mask = ButtonPressMask;
  attrs.override_redirect = 1;

  Window window = XCreateWindow(dpy, XRootWindowOfScreen(screen), win_x, win_y,
                                win_w, win_h, bor_w, CopyFromParent,
                                InputOutput, CopyFromParent, mask, &attrs);

  ASSERT(window && "_create_notification_window: failed to create window.");

  XSelectInput(dpy, window, ButtonPressMask);

  return window;
}

static void _draw_indicator(Display *dpy, Notification *notification) {

  const char delim = *notification->indicator;
  const char *end = strchrnul(notification->frame, delim);

  XftDrawStringUtf8(notification->draw, notification->ind_color,
                    notification->ind_font, notification->ind_x,
                    notification->ind_y, (FcChar8 *)notification->frame,
                    (size_t)(end - notification->frame));

  if (*(end + 1) == delim)
    notification->frame = notification->indicator + 1;
  else
    notification->frame = end + 1;
}

void notification_draw(Display *dpy, Notification *notification) {

  XClearWindow(dpy, notification->window);

  _draw_indicator(dpy, notification);

  XftDrawStringUtf8(notification->draw, notification->txt_color,
                    notification->txt_font, notification->txt_x,
                    notification->txt_y, (FcChar8 *)notification->text,
                    strlen(notification->text));

  clock_gettime(CLOCK_MONOTONIC, &notification->start_time);
  notification->last_time = notification->start_time;
}

void notification_open(Display *dpy, Config *config,
                       Notification *notification) {

  ASSERT(notification->indicator && "notification_open: indicator not set");
  ASSERT(notification->ind_font && "notification_open: ind_font not set");
  ASSERT(notification->ind_color && "notification_open: ind_color not set");
  ASSERT(notification->text && "notification_open: text not set");
  ASSERT(notification->txt_font && "notification_open: txt_font not set");
  ASSERT(notification->txt_color && "notification_open: txt_color not set");

  notification->frame = notification->indicator + 1;

  notification->window = _create_notification_window(
      dpy, notification->win_x, notification->win_y, notification->win_w,
      notification->win_h, config->border_size, config->bg_color,
      config->border_color);

  int scr_nbr = DefaultScreen(dpy);

  notification->draw =
      XftDrawCreate(dpy, notification->window, DefaultVisual(dpy, scr_nbr),
                    DefaultColormap(dpy, scr_nbr));

  notification_map(dpy, notification);
  notification_draw(dpy, notification);
}

void notification_map(Display *dpy, Notification *notification) {

  XMapWindow(dpy, notification->window);
}

void notification_move(Display *dpy, Notification *notification) {

  XMoveWindow(dpy, notification->window, notification->win_x,
              notification->win_y);
}

void notification_close(Display *dpy, Notification *notification) {

  XftDrawDestroy(notification->draw);
  XDestroyWindow(dpy, notification->window);
}

bool notification_update(Display *dpy, Notification *notification) {

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  long elapsed = (now.tv_sec - notification->start_time.tv_sec) * 1000 +
                 (now.tv_nsec - notification->start_time.tv_nsec) / 1000000;

  if (XPending(dpy)) {
    XEvent e;
    if (XCheckWindowEvent(dpy, notification->window, ButtonPressMask, &e)) {
      if (e.xbutton.button == Button1) {
        XUnmapWindow(dpy, e.xbutton.window);
        return false;
      }
    }
  }

  if (notification->timeout > 0 && elapsed >= notification->timeout * 1000) {
    XUnmapWindow(dpy, notification->window);
    return false;
  }

  if (notification->type == UNOT_TYPE_SPINNER) {

    elapsed = (now.tv_sec - notification->last_time.tv_sec) * 1000 +
              (now.tv_nsec - notification->last_time.tv_nsec) / 1000000;

    if (elapsed >= 150) {

      XClearArea(dpy, notification->window, 0, 0, notification->txt_x, 0,
                 False);

      _draw_indicator(dpy, notification);

      notification->last_time = now;
    }
  }

  return true;
}
