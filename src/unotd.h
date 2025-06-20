#ifndef UNOT_UNOTD_H
#define UNOT_UNOTD_H

#include <assert.h>

#include "config.h"
#include "list.h"
#include "notification.h"

typedef struct _Unotd {

  Display *display;
  Config config;

  XftFont *msg_font;
  XftColor msg_color;
  XftColor ind_color;

  NotificationNode *head_open;
  NotificationNode *head_waiting;
} Unotd;

XftFont *unotd_resolve_indicator_font(Display *dpy, Config *config,
                                      const char *indicator);

void unotd_handle_events(Unotd *unotd);
void unotd_update_notifications(Unotd *unotd);
void unotd_handle_unmapped_notification(Unotd *unotd, Window window);
void unotd_allocate_ext_resources(Unotd *unotd, Notification *notification);
void unotd_free_ext_resources(Unotd *unotd, Notification *notification);

#endif
