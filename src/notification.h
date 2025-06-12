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

  NotificationType type;
  union {

    struct {
      Spinner *spinner;
      uint8_t current_frame;
    };

    Icon *icon;
  };
  uint8_t i_x, i_y;

  const char *messsage;
  uint8_t t_x, t_y;

  XftFont *font;
  XftColor color;
  uint8_t timeout;
  struct timespec last_updated;

} Notification;

Notification *OpenNotification(Display *dpy, Config *config,
                                       XftFont *font, const char *message,
                                       NotificationType type,
                                       void *indicator, uint8_t timeout);

void UpdateNotification(Display *dpy, Notification *notification);

// void CloseNotification(Display *dpy, Notification *notification);

#endif
