#ifndef MICRONOT_NOTIFICATION_H
#define MICRONOT_NOTIFICATION_H

#include "config.h"
#include "indicator.h"
#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum _NotificationState : u_int16_t {
  UNOT_NEED_INIT,
  UNOT_NEED_REOPEN,
  UNOT_NEED_REDRAW,
  UNOT_NEED_UPDATE
} NotificationState;

typedef struct _Notification {

  uint64_t client_id; // 8

  Window window; // 8
  XftDraw *draw; // 8

  char *txt;           // 8
  XftFont *txt_font;   // 8
  XftColor *txt_color; // 8

  Indicator ind; // 40

  unsigned long timeout;                 // 8
  struct timespec start_time, last_time; // 16

  NotificationState state; // 2
  bool custom_txt_font;    // 1
  bool custom_txt_color;   // 1

  uint16_t txt_x, txt_y; // 4
  uint16_t win_x, win_y; // 4
  uint16_t win_w, win_h; // 4

  bool is_persistent; // 1

} Notification;

typedef Window NotificationID;

void notification_open(Display *dpy, Config *config,
                       Notification *notification);
void notification_map(Display *dpy, Notification *notification);
void notification_draw(Display *dpy, Notification *notification);
void notification_move(Display *dpy, Notification *notification);
void notification_resize(Display *dpy, Notification *notification);
void notification_move_resize(Display *dpy, Notification *notification);
bool notification_update(Display *dpy, Notification *notification);
void notification_close(Display *dpy, Notification *notification);

#endif
