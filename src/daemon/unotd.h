#ifndef MICRONOT_UNOTD_H
#define MICRONOT_UNOTD_H

#include "nlist.h"

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

#endif
