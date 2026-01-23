#include "server.h"
#include "../protocol.h"
#include "command.h"
#include "utils.h"
#include <assert.h>
#include <poll.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static void _pollset_init(PollSet *set) {

  set->fds = malloc(sizeof(struct pollfd) * UNOT_MIN_PFDS);
  set->capacity = UNOT_MIN_PFDS;
  set->count = 0;
}

static void _pollset_add_pfd(PollSet *set, int newfd) {

  if (set->count == set->capacity) {
    set->capacity *= 2;
    set->fds = realloc(set->fds, sizeof(struct pollfd) * set->capacity);
  }

  set->fds[set->count] =
      (struct pollfd){.fd = newfd, .events = POLLIN, .revents = 0};

  set->count++;
}

static void _pollset_del_pfd(PollSet *set, int idx) {

  set->fds[idx] = set->fds[set->count - 1];
  set->count--;

  if (set->count <= set->capacity / 4 && set->capacity > UNOT_MIN_PFDS) {
    set->capacity /= 2;
    set->fds = realloc(set->fds, sizeof(struct pollfd) * set->capacity);
  }
}

static int _get_listener() {

  int sockfd;
  struct sockaddr_un addr;

  if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  unlink(UNOT_SOCK_PATH);

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, UNOT_SOCK_PATH, sizeof(addr.sun_path) - 1);

  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind");
    close(sockfd);
    exit(1);
  }

  if (listen(sockfd, 5) == -1) {
    perror("listen");
    close(sockfd);
    exit(1);
  }

  return sockfd;
}

void server_init(ServerCtx *sctx) {

  sctx->listener = _get_listener();
  assert(sctx->listener && "server_init: failed getting listener");
  _pollset_init(&sctx->set);
  _pollset_add_pfd(&sctx->set, sctx->listener);
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

        switch (ncurr->ind.type) {

        case INDICATOR_TYPE_ICON:
          notification_close(sctx->display, ncurr);
          nlist_remove(&sctx->open, prev);
          break;

        case INDICATOR_TYPE_SPINNER:
          nlist_unlink(&sctx->open, prev);
          nlist_append(&sctx->wait, curr);
          break;
        }

        curr = prev ? prev->next : sctx->open.head;
        continue;
      }
    }

    prev = curr;
    curr = curr->next;
  }
}

void server_process_connections(ServerCtx *sctx) {

  PollSet *set = &sctx->set;

  for (int i = 0; i < set->count; i++) {

    if (set->fds[i].revents & (POLLIN | POLLHUP)) {

      if (set->fds[i].fd == sctx->listener) {
        int newfd = accept(sctx->listener, NULL, NULL);
        if (newfd == -1) {
          // TODO
        }
        _pollset_add_pfd(set, newfd);
      }

      else {

        char buf[256];
        ProtocolBuffer pbuf = {.buf = buf, .state = buf, .len = sizeof(buf)};

        int fd = set->fds[i].fd;
        if (!protocol_recv(fd, &pbuf)) {
          close(fd);
          _pollset_del_pfd(set, i);
          i--;
          continue;
        }

        command_handle(sctx, fd, &pbuf);
      }
    }
  }
}
