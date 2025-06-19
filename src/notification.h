#ifndef UNOT_NOTIFICATION_H
#define UNOT_NOTIFICATION_H

#include "config.h"
#include "icon.h"
#include "spinner.h"

#include <X11/Xlib.h>
#include <time.h>

typedef enum _NotificationType {

  UNOT_MESSAGE,
  UNOT_SPINNER

} NotificationType;

typedef struct _Notification {

  Window window;
  XftDraw *draw;

  const char *indicator;
  XftFont *ind_font;
  XftColor *ind_color;
  uint8_t ind_x, ind_y;

  const char *message;
  XftFont *msg_font;
  XftColor *msg_color;
  uint8_t msg_x, msg_y;

  NotificationType type;
  const char *frame;
  int timeout;

  struct timespec last_updated;

} Notification;

void notification_open(Display *dpy, Config *config,
                       Notification *notification);

void notification_update(Display *dpy, Notification *notification);

void notification_close(Display *dpy, Notification *notification);

#endif
