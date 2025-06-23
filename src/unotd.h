#ifndef UNOT_UNOTD_H
#define UNOT_UNOTD_H

#include <assert.h>

#include "list.h"
#include "notification.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef enum _Origin {

  UNOT_ORIGIN_TOP_RIGHT,
  UNOT_ORIGIN_TOP_LEFT,
  UNOT_ORIGIN_BOTTOM_RIGHT,
  UNOT_ORIGIN_BOTTOM_LEFT

} Origin;

typedef struct _Unotd {

  Display *display;
  Config config;

  Origin origin;
  XftFont *msg_font;
  XftColor msg_color;
  XftColor ind_color;

  NotificationList open;
  NotificationList wait;
} Unotd;

void unotd_handle_events(Unotd *unotd);
void unotd_update_notifications(Unotd *unotd);
void unotd_calc_notification_layout(Unotd *unotd, Notification *notification);
void unotd_reposition_notification(Unotd *unotd, NotificationNode *previous,
                                   NotificationNode *current);
void unotd_handle_unmapped_notification(Unotd *unotd, Window window);
XftFont *unotd_resolve_indicator_font(Unotd *unotd, const char *indicator);
void unotd_allocate_ext_resources(Unotd *unotd, Notification *notification);
void unotd_free_ext_resources(Unotd *unotd, Notification *notification);

#endif
