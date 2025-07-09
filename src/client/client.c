#include <errno.h>
#include <micronot/client.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int unot_get_connection(const char *sock_path) {

  int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("ERROR: socket");
    return -1;
  }

  struct sockaddr_un addr = {0};
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);

  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) ==
      -1) {
    perror("ERROR: connect");
    close(sockfd);
    return -1;
  }

  return sockfd;
}

static bool _send_buffer(int conn, const char *cmdbuf, size_t size) {

  size_t sent = 0;
  while (sent < size) {
    ssize_t n = send(conn, cmdbuf + sent, size - sent, 0);
    if (n < 0) {
      if (errno == EINTR)
        continue;
      perror("ERROR: send");
      return false;
    }

    else if (n == 0) {
      fprintf(stderr, "ERROR: send() returned 0\n");
      return false;
    }
    sent += (size_t)n;
  }
  return true;
}

bool unot_notify_message(int conn, const char *text, const char *ind_str,
                         const char *txt_font, unsigned long timeout,
                         unsigned long ind_fg_color,
                         unsigned long txt_fg_color) {

  if (!(text && ind_str))
    return false;

  size_t size = 256;
  char cmdbuf[size];
  size_t offset = 0;

  offset += snprintf(cmdbuf + offset, size - offset, "MSG\n");
  offset += snprintf(cmdbuf + offset, size - offset, "txt:%s\n", text);
  offset += snprintf(cmdbuf + offset, size - offset, "ind:%s\n", ind_str);

  if (txt_font) {
    offset += snprintf(cmdbuf + offset, size - offset, "tfn:%s\n", txt_font);
  }

  if (timeout) {
    offset += snprintf(cmdbuf + offset, size - offset, "tim:%lu\n", timeout);
  }

  if (ind_fg_color) {
    offset +=
        snprintf(cmdbuf + offset, size - offset, "ifg:%lx\n", ind_fg_color);
  }

  if (txt_fg_color) {
    offset +=
        snprintf(cmdbuf + offset, size - offset, "tfg:%lx\n", txt_fg_color);
  }

  offset += snprintf(cmdbuf + offset, size - offset, "\n");

  if (!_send_buffer(conn, cmdbuf, offset))
    return false;

  return true;
}
