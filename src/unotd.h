#ifndef UNOT_UNOTD_H
#define UNOT_UNOTD_H

#include "nlist.h"
#include "notification.h"

typedef struct _PollSet {

  struct pollfd *fds;
  size_t capacity;
  size_t count;

} PollSet;

typedef struct _Unotd {

  Display *display;

  Config config;

  int listener;
  PollSet set;

  XftFont *txt_font;
  XftColor txt_color;
  XftColor ind_color;

  NotificationList open;
  NotificationList wait;

  pthread_mutex_t nlist_lock;
  pthread_cond_t nlist_empty;
  pthread_cond_t notif_open;

} Unotd;

void unotd_update_notifications(Unotd *unotd);
void unotd_init_notification_resources(Unotd *unotd, Notification *target);
void unotd_free_notification_resources(Unotd *unotd, Notification *target);

#endif
