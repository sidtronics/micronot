#ifndef MICRONOT_NOTIFICATION_H
#define MICRONOT_NOTIFICATION_H

#include "config.h"

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdbool.h>
#include <time.h>

typedef enum _NotificationType {
  UNOT_TYPE_MESSAGE,
  UNOT_TYPE_SPINNER
} NotificationType;

typedef enum _NotificationState {
  UNOT_NEED_INIT,
  UNOT_NEED_REOPEN,
  UNOT_NEED_REDRAW,
  UNOT_NEED_UPDATE
} NotificationState;

typedef struct _Notification {

  Window window;
  u_int16_t win_x, win_y;
  u_int16_t win_w, win_h;

  XftDraw *draw;

  char *indicator;
  XftFont *ind_font;
  XftColor *ind_color;
  u_int16_t ind_x, ind_y;

  char *text;
  XftFont *txt_font;
  XftColor *txt_color;
  u_int16_t txt_x, txt_y;

  NotificationState state;
  NotificationType type;
  const char *frame;
  unsigned long timeout;

  struct timespec start_time;
  struct timespec last_time;

} Notification;

void notification_open(Display *dpy, Config *config,
                       Notification *notification);

void notification_map(Display *dpy, Notification *notification);

void notification_draw(Display *dpy, Notification *notification);

void notification_move(Display *dpy, Notification *notification);

bool notification_update(Display *dpy, Notification *notification);

void notification_close(Display *dpy, Notification *notification);

#endif
