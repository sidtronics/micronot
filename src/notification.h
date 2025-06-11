#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "config.h"
#include "icon.h"
#include "spinner.h"

#include <X11/Xlib.h>
#include <time.h>

typedef enum _UnotNotificationType {

  UNOT_MESSAGE,
  UNOT_SPINNER

} UnotNotificationType;

typedef struct _UnotNotification {

  Window window;
  XftDraw *draw;

  UnotNotificationType type;
  union {

    struct {
      UnotSpinner *spinner;
      uint8_t current_frame;
    };

    UnotIcon *icon;
  };
  uint8_t i_x, i_y;

  const char *messsage;
  uint8_t t_x, t_y;

  XftFont *font;
  XftColor color;
  uint8_t timeout;
  struct timespec last_updated;

} UnotNotification;

UnotNotification *UnotOpenNotification(Display *dpy, UnotConfig *config,
                                       XftFont *font, const char *message,
                                       UnotNotificationType type,
                                       void *indicator, uint8_t timeout);

void UnotUpdateNotification(Display *dpy, UnotNotification *notification);

// void UnotCloseNotification(Display *dpy, UnotNotification *notification);

#endif
