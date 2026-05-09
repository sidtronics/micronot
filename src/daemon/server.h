#ifndef MICRONOT_SERVER_H
#define MICRONOT_SERVER_H

#include "../hf.h"
#include "nlist.h"
#define UNOT_MAX_CONNECTIONS 10
#define UNOT_SOCK_PATH "/tmp/unotd.sock"

typedef struct _Client {
    ClientID id;
    hf_context ctx;
} Client;

typedef struct _ServerCtx {

  Display *display;

  Config config;

  struct pollfd pfds[1 + UNOT_MAX_CONNECTIONS]; // First element stores listener
  Client clients[UNOT_MAX_CONNECTIONS];

  XftFont *txt_font;
  XftColor txt_color;
  XftColor ind_color;

  NotificationList open;
  NotificationList wait;

  pthread_mutex_t nlist_lock;
  pthread_cond_t nlist_empty;
  pthread_cond_t notif_open;

  ClientID next_client_id;

} ServerCtx;

void server_init(ServerCtx *sctx);
void *server_handle(void *arg);
void server_process_connections(ServerCtx *sctx);
void server_update_notifications(ServerCtx *sctx);

#endif
