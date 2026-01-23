#ifndef MICRONOT_SERVER_H
#define MICRONOT_SERVER_H

#include "nlist.h"
#define UNOT_MIN_PFDS 10
#define UNOT_SOCK_PATH "/tmp/unotd.sock"

typedef struct _PollSet {
  struct pollfd *fds;
  size_t capacity;
  size_t count;
} PollSet;

typedef struct _ServerCtx {

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

} ServerCtx;

void server_init(ServerCtx *sctx);
void server_update_notifications(ServerCtx *sctx);
void server_process_connections(ServerCtx *sctx);

#endif
