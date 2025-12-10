#ifndef MICRONOT_NOTIFICATION_H
#define MICRONOT_NOTIFICATION_H

#include "config.h"
#include "indicator.h"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdbool.h>

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

  Indicator ind;
  u_int16_t ind_x, ind_y;

  char *txt;
  XftFont *txt_font;
  XftColor *txt_color;
  u_int16_t txt_x, txt_y;

  NotificationState state;
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
