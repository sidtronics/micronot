#include "command.h"
#include "utils.h"
#include <assert.h>
#include <poll.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "../hf_defs.h"
#define HF_IMPLEMENTATION
#include "server.h"

void *server_handle(void *arg) {

  ServerCtx *sctx = (ServerCtx *)arg;

  while (1) {

    if (!hf_poll_sync(sctx->pfds, UNOT_MAX_CONNECTIONS)) {
      perror("poll");
      exit(1);
    }

    server_process_connections(sctx);
  }

  return NULL;
}

static int _get_listener() {

  int fd;
  struct sockaddr_un addr;

  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  unlink(UNOT_SOCK_PATH);

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, UNOT_SOCK_PATH, sizeof(addr.sun_path) - 1);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind");
    close(fd);
    exit(1);
  }

  if (listen(fd, 5) == -1) {
    perror("listen");
    close(fd);
    exit(1);
  }

  return fd;
}

void server_init(ServerCtx *sctx) {

  sctx->pfds[0] =
      (struct pollfd){.fd = _get_listener(), .events = POLLIN, .revents = 0};

  for (int i = 0; i < UNOT_MAX_CONNECTIONS; i++) {
    sctx->pfds[i + 1].fd = -1;
  }
}

static bool _alloc_connection(ServerCtx *sctx, int fd) {

  for (int i = 1; i < UNOT_MAX_CONNECTIONS; i++) {
    if (sctx->pfds[i].fd == -1) {
      sctx->pfds[i] = (struct pollfd){.fd = fd, .events = POLLIN, .revents = 0};
      hf_clear_error(&sctx->ctxs[i - 1]);
      hf_clear_buffer(&sctx->ctxs[i - 1]);
      return true;
    }
  }

  return false;
}

static void _accept_new_connection(ServerCtx *sctx) {

  int fd = accept(sctx->pfds[0].fd, NULL, NULL);
  if (fd == -1) {
    perror("accept");
    return;
  }

  if (!_alloc_connection(sctx, fd)) {
    fprintf(stderr, "[unotd:server]: max connections reached\n");
    close(fd);
  }
}

static void _close_connection(ServerCtx *sctx, int idx) {

  struct pollfd *pfd = &sctx->pfds[idx];
  close(pfd->fd);
  pfd->fd = -1;
}

static void _handle_event_recv(ServerCtx *sctx, int idx) {

  struct pollfd *pfd = &sctx->pfds[idx];
  hf_context *ctx = &sctx->ctxs[idx - 1];

  if (!hf_message_recv_async(pfd->fd, ctx)) {
    fprintf(stderr, "[unotd:server]: %s\n", hf_get_error_string(ctx));
    _close_connection(sctx, idx);
    return;
  }

  if (hf_message_received(ctx)) {

    hf_message msg = {0};

    if (hf_message_parse(ctx, &msg)) {
      command_handle(sctx, &msg);
    }

    else {
      fprintf(stderr, "[unotd:server]: %s\n", hf_get_error_string(ctx));
      hf_clear_error(ctx);
      hf_message_set_header(&msg, UNOT_H_ERROR);
    }

    hf_message_build(ctx, &msg);
    pfd->events = POLLOUT;
  }
}

static void _handle_event_send(ServerCtx *sctx, int idx) {

  struct pollfd *pfd = &sctx->pfds[idx];
  hf_context *ctx = &sctx->ctxs[idx - 1];

  if (!hf_message_send_async(pfd->fd, ctx)) {
    fprintf(stderr, "[unotd:server]: %s\n", hf_get_error_string(ctx));
    _close_connection(sctx, idx);
    return;
  }

  if (hf_message_sent(ctx)) {
    hf_clear_buffer(ctx);
    pfd->events = POLLIN;
  }
}

void server_process_connections(ServerCtx *sctx) {

  for (int i = 0; i <= UNOT_MAX_CONNECTIONS; i++) {

    short rev = sctx->pfds[i].revents;

    if (rev & (POLLERR | POLLNVAL)) {
      _close_connection(sctx, i);
      continue;
    }

    if (i == 0 && (rev & POLLIN)) {
      _accept_new_connection(sctx);
      continue;
    }

    if (rev & POLLIN) {
      _handle_event_recv(sctx, i);
    }

    else if (rev & POLLOUT) {
      _handle_event_send(sctx, i);
    }
  }
}

void server_update_notifications(ServerCtx *sctx) {

  bool needs_reposition = false;
  NotificationNode *prev = NULL;
  NotificationNode *curr = sctx->open.head;
  while (curr) {

    Notification *nprev = prev ? &prev->notification : NULL;
    Notification *ncurr = &curr->notification;

    switch (ncurr->state) {

    case UNOT_NEED_INIT:
      utils_calculate_notification_layout(sctx->display, &sctx->config, ncurr);
      utils_reposition_notification(sctx->display, &sctx->config, nprev, ncurr);
      notification_open(sctx->display, &sctx->config, ncurr);
      pthread_cond_signal(&sctx->notif_open);
      ncurr->state = UNOT_NEED_UPDATE;
      break;

    case UNOT_NEED_REDRAW:
      needs_reposition = true;
      [[fallthrough]];
    case UNOT_NEED_REOPEN:
      utils_reposition_notification(sctx->display, &sctx->config, nprev, ncurr);
      notification_move(sctx->display, ncurr);
      notification_map(sctx->display, ncurr);
      notification_draw(sctx->display, ncurr);
      ncurr->state = UNOT_NEED_UPDATE;
      break;

    case UNOT_NEED_UPDATE:

      if (needs_reposition) {
        utils_reposition_notification(sctx->display, &sctx->config, nprev,
                                      ncurr);
        notification_move(sctx->display, ncurr);
      }

      if (notification_update(sctx->display, ncurr) == 0) {

        needs_reposition = true;

        if (ncurr->ind.frame_count > 1) {
          nlist_unlink(&sctx->open, prev);
          nlist_append(&sctx->wait, curr);
        }

        else {
          notification_close(sctx->display, ncurr);
          nlist_remove(&sctx->open, prev);
        }

        curr = prev ? prev->next : sctx->open.head;
        continue;
      }
    }

    prev = curr;
    curr = curr->next;
  }
}
