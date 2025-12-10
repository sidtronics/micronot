#include "server.h"
#include "../protocol.h"
#include "command.h"
#include <poll.h>
#include <assert.h>
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

void server_init(Unotd *unotd) {

  unotd->listener = _get_listener();
  assert(unotd->listener && "server_init: failed getting listener");
  _pollset_init(&unotd->set);
  _pollset_add_pfd(&unotd->set, unotd->listener);
}

void server_process_connections(Unotd *unotd) {

  PollSet *set = &unotd->set;

  for (int i = 0; i < set->count; i++) {

    if (set->fds[i].revents & (POLLIN | POLLHUP)) {

      if (set->fds[i].fd == unotd->listener) {
        int newfd = accept(unotd->listener, NULL, NULL);
        if (newfd == -1) {
          // TODO
        }
        _pollset_add_pfd(set, newfd);
      }

      else {

        char buf[256];
        ProtocolBuffer pbuf = ProtocolBufferInit(buf);
        int fd = set->fds[i].fd;
        if (!protocol_recv(fd, &pbuf)) {
          close(fd);
          _pollset_del_pfd(set, i);
          i--;
          continue;
        }

        command_handle(unotd, fd, &pbuf);
      }
    }
  }
}
