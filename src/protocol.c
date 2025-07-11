#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

bool protocol_recv_block(int fd, char *buf, size_t len) {

  char *p = buf;
  size_t bytes_read = 0;
  do {

    if (bytes_read >= len - 1) {
      fprintf(stderr, "[protocol] ERROR: block too large\n");
      return false;
    }

    ssize_t n = recv(fd, buf + bytes_read, len - bytes_read - 1, 0);

    if (n <= 0) {
      if (n < 0) {
        perror("[protocol] ERROR: recv");
        return false;
      }
      fprintf(stderr, "[protocol] INFO: connection closed by the peer\n");
      return false;
    }

    if (bytes_read != 0)
      p = buf + bytes_read - 1;

    bytes_read += n;

  } while (!strstr(p, "\n\n"));

  buf[bytes_read] = 0;
  return true;
}

bool protocol_send_block(int fd, char *buf, size_t len) {

  const char *end = strstr(buf, "\n\n");
  if (!end) {
    fprintf(stderr, "[protocol] ERROR: block not terminated by blank line\n");
    return false;
  }

  size_t total_bytes = (end - buf) + 2;

  size_t bytes_sent = 0;
  while (bytes_sent < total_bytes) {

    ssize_t n = send(fd, buf + bytes_sent, total_bytes - bytes_sent, 0);

    if (n <= 0) {
      if (n < 0) {
        perror("[protocol] ERROR: send");
        return false;
      }
      fprintf(stderr, "[protocol] ERROR: send() returned 0\n");
      return false;
    }

    bytes_sent += (size_t)n;
  }
  return true;
}
