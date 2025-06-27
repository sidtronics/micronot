#ifndef UNOT_UNOTD_H
#define UNOT_UNOTD_H

#include "list.h"
#include "notification.h"

typedef struct _Unotd {

  Display *display;

  Config config;

  XftFont *msg_font;
  XftColor msg_color;
  XftColor ind_color;

  NotificationList open;
  NotificationList wait;
} Unotd;

void unotd_handle_events(Unotd *unotd);
void unotd_handle_unmap(Unotd *unotd, Window window);
bool unotd_match_window(Notification *node, void *data);
void unotd_update_visitor(Notification *prev, Notification *curr, void *data);
void unotd_reposition_visitor(Notification *prev, Notification *curr,
                              void *data);

void unotd_init_notification_resources(Unotd *unotd, Notification *target);
void unotd_free_notification_resources(Unotd *unotd, Notification *target);

#endif
