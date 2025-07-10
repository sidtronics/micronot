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

static bool _send_buffer(int conn, const char *buf, size_t len) {

  size_t sent = 0;
  while (sent < len) {
    ssize_t n = send(conn, buf + sent, len - sent, 0);
    if (n < 0) {
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

static bool _recv_buffer(int conn, char *buf, size_t len) {

  char *p = buf;
  size_t bytes_read = 0;
  do {

    if (bytes_read >= len - 1) {

      fprintf(stderr, "error: response too large\n");
      send(conn, "ERROR\n\n", 7, 0);
      return false;
    }

    ssize_t n = recv(conn, buf + bytes_read, len - bytes_read - 1, 0);

    if (n <= 0) {

      if (n < 0) {
        perror("recv");
      }

      fprintf(stderr, "info: client closed the connection\n");
      return false;
    }

    if (bytes_read != 0)
      p = buf + bytes_read - 1;

    bytes_read += n;

  } while (strstr(p, "\n\n") == NULL);

  return true;
}

bool unot_notify_message(int conn, const char *text, u_int16_t mask,
                         NotificationAttributes *attrs) {

  if (!text || (!attrs->indicator && !attrs->indicator_name))
    return false;

  size_t size = 256;
  char buf[size];
  size_t offset = 0;

  offset += snprintf(buf + offset, size - offset, "MSG\n");
  offset += snprintf(buf + offset, size - offset, "txt:%s\n", text);

  if (mask & UNIndicator)
    offset +=
        snprintf(buf + offset, size - offset, "ind:%s\n", attrs->indicator);

  else
    offset += snprintf(buf + offset, size - offset, "nme:%s\n",
                       attrs->indicator_name);

  if (mask & UNTextFG) {
    offset +=
        snprintf(buf + offset, size - offset, "tfg:%lx\n", attrs->text_fg);
  }

  if (mask & UNIndicatorFG) {
    offset +=
        snprintf(buf + offset, size - offset, "ifg:%lx\n", attrs->indicator_fg);
  }

  if (mask & UNTextFont) {
    offset +=
        snprintf(buf + offset, size - offset, "tfn:%s\n", attrs->text_font);
  }

  if (mask & UNTimeout) {
    offset +=
        snprintf(buf + offset, size - offset, "tim:%lu\n", attrs->timeout);
  }

  offset += snprintf(buf + offset, size - offset, "\n");

  if (!_send_buffer(conn, buf, offset))
    return false;

  if (!_recv_buffer(conn, buf, sizeof(buf)) || strncmp(buf, "OK", 2) != 0) {
    printf("fked");
    return false;
  }

  return true;
}
