#ifndef UNOT_NOTIFICATION_H
#define UNOT_NOTIFICATION_H

#include "config.h"

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <time.h>

typedef enum _NotificationType {

  UNOT_MESSAGE,
  UNOT_SPINNER

} NotificationType;

typedef struct _Notification {

  Window window;
  u_int16_t win_x, win_y;
  u_int16_t win_w, win_h;

  XftDraw *draw;

  const char *indicator;
  XftFont *ind_font;
  XftColor *ind_color;
  u_int16_t ind_x, ind_y;

  const char *message;
  XftFont *msg_font;
  XftColor *msg_color;
  u_int16_t msg_x, msg_y;

  NotificationType type;
  const char *frame;
  int timeout;

  struct timespec start_time;
  struct timespec last_time;

} Notification;

void notification_open(Display *dpy, Config *config,
                       Notification *notification);

void notification_update(Display *dpy, Notification *notification);

void notification_close(Display *dpy, Notification *notification);

#endif
