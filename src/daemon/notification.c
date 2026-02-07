#include "notification.h"
#include "utils.h"
#include <assert.h>
#include <time.h>

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

  assert(window && "_create_notification_window: failed to create window.");

  XSelectInput(dpy, window, ButtonPressMask);

  return window;
}

void notification_draw(Display *dpy, Notification *notification) {

  XClearWindow(dpy, notification->window);

  const char delim = *notification->ind.str;
  const char *end = strchr(notification->ind.frame, delim);

  XftDrawStringUtf8(notification->draw, notification->ind.color,
                    notification->ind.font, notification->ind.x,
                    notification->ind.y, (FcChar8 *)notification->ind.frame,
                    (size_t)(end - notification->ind.frame));

  XftDrawStringUtf8(notification->draw, notification->txt_color,
                    notification->txt_font, notification->txt_x,
                    notification->txt_y, (FcChar8 *)notification->txt,
                    strlen(notification->txt));

  clock_gettime(CLOCK_MONOTONIC, &notification->start_time);
  notification->last_time = notification->start_time;
}

void notification_open(Display *dpy, Config *config,
                       Notification *notification) {

  assert(notification->ind.str && "notification_open: indicator not set");
  assert(notification->ind.font && "notification_open: ind_font not set");
  assert(notification->ind.color && "notification_open: ind_color not set");
  assert(notification->txt && "notification_open: text not set");
  assert(notification->txt_font && "notification_open: txt_font not set");
  assert(notification->txt_color && "notification_open: txt_color not set");

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

  free(notification->txt);
  indicator_free_str(dpy, &notification->ind);

  if (notification->custom_txt_font)
    XftFontClose(dpy, notification->txt_font);

  if (notification->custom_txt_color)
    utils_deallocate_color(dpy, notification->txt_color);

  if (notification->ind.custom_color)
    utils_deallocate_color(dpy, notification->ind.color);
}

bool notification_update(Display *dpy, Notification *notification) {

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  unsigned long elapsed_ms =
      (now.tv_sec - notification->start_time.tv_sec) * 1000 +
      (now.tv_nsec - notification->start_time.tv_nsec) / 1000000;

  XEvent e;
  if (XCheckWindowEvent(dpy, notification->window, ButtonPressMask, &e)) {
    if (e.xbutton.button == Button1) {
      XUnmapWindow(dpy, notification->window);
      return false;
    }
  }

  if (notification->timeout > 0 && elapsed_ms >= notification->timeout * 1000) {
    XUnmapWindow(dpy, notification->window);
    return false;
  }

  if (notification->ind.frame_count > 1) {

    elapsed_ms = (now.tv_sec - notification->last_time.tv_sec) * 1000 +
                 (now.tv_nsec - notification->last_time.tv_nsec) / 1000000;

    if (elapsed_ms >= 150) {

      size_t frame_size = indicator_step_frame(&notification->ind);

      XClearArea(dpy, notification->window, 0, 0, notification->txt_x, 0,
                 False);

      XftDrawStringUtf8(notification->draw, notification->ind.color,
                        notification->ind.font, notification->ind.x,
                        notification->ind.y, (FcChar8 *)notification->ind.frame,
                        frame_size);

      notification->last_time = now;
    }
  }

  return true;
}
