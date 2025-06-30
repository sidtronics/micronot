#ifndef UNOT_UNOTD_H
#define UNOT_UNOTD_H

#include "list.h"
#include "notification.h"
#include <semaphore.h>

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

  XftFont *msg_font;
  XftColor msg_color;
  XftColor ind_color;

  NotificationList open;
  NotificationList wait;

  pthread_cond_t notification_opened;
  sem_t notification_count;

} Unotd;

void unotd_handle_events(Unotd *unotd);
void unotd_handle_unmap(Unotd *unotd, Window window);

void unotd_update_visitor(Notification *prev, Notification *curr, void *data);
void unotd_reposition_visitor(Notification *prev, Notification *curr,
                              void *data);

void unotd_init_notification_resources(Unotd *unotd, Notification *target);
void unotd_free_notification_resources(Unotd *unotd, Notification *target);

#endif
