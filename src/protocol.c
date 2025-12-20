#include "protocol.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

bool protocol_recv(int fd, ProtocolBuffer *pbuf) {

  char *recv_beg = pbuf->buf;
  char *recv_end;
  ssize_t n = 0;
  do {

    recv_beg += n;
    size_t rem = (pbuf->buf + pbuf->len) - recv_beg;
    if (rem == 0) {
      fprintf(stderr, "[protocol] ERROR: block too large\n");
      return false;
    }

    n = recv(fd, recv_beg, rem, 0);

    if (n <= 0) {
      if (n < 0)
        perror("[protocol] ERROR: recv");
      else
        fprintf(stderr, "[protocol] INFO: connection closed by the peer\n");
      return false;
    }

    recv_end = memchr(recv_beg, '\n', n);

  } while (recv_end == NULL);

  *recv_end = 0;
  pbuf->state = pbuf->buf;
  return true;
}

bool protocol_send(int fd, ProtocolBuffer *pbuf) {

  size_t size = strnlen(pbuf->buf, pbuf->len - 1);
  pbuf->buf[size] = '\n';
  size++;

  size_t bytes_sent = 0;
  while (bytes_sent < size) {

    ssize_t n = send(fd, pbuf->buf + bytes_sent, size - bytes_sent, 0);

    if (n <= 0) {
      if (n < 0)
        perror("[protocol] ERROR: send");
      else
        fprintf(stderr, "[protocol] ERROR: send() returned 0\n");
      return false;
    }

    bytes_sent += (size_t)n;
  }

  return true;
}

bool protocol_parse_header(ProtocolBuffer *pbuf, const char **cmd) {

  pbuf->state = strchr(pbuf->buf, ' ');

  if (pbuf->state)
    *pbuf->state++ = '\0';

  *cmd = pbuf->buf;
  return true;
}

bool protocol_parse_field(ProtocolBuffer *pbuf, const char **key,
                          const char **val) {

  // TODO: handle '\' as escaping character to allow use of double quotes.
  // TODO: handle empty keys and values.

  if (!pbuf->state)
    return false;

  *key = pbuf->state;
  pbuf->state = strchr(pbuf->state, ':');
  if (!pbuf->state) {
    fprintf(stderr, "[protocol] ERROR: malformed pair, expected ':'\n");
    *key = NULL;
    *val = NULL;
    return true;
  }

  *pbuf->state++ = '\0';

  if (*pbuf->state == '"') {

    *val = ++pbuf->state;
    pbuf->state = strchr(pbuf->state, '"');
    if (!pbuf->state) {
      fprintf(stderr, "[protocol] ERROR: missing closing '\"''\n");
      *key = NULL;
      *val = NULL;
      return true;
    }

    *pbuf->state++ = '\0';
  }

  else
    *val = pbuf->state;

  pbuf->state = strchr(pbuf->state, ' ');
  if (pbuf->state)
    *pbuf->state++ = '\0';

  return true;
}

bool protocol_append_header(ProtocolBuffer *pbuf, char *cmd) {

  pbuf->state = pbuf->buf;
  int n = snprintf(pbuf->state, pbuf->len, "%s", cmd);
  if (n < 0 || (size_t)n >= pbuf->len)
    return false;
  pbuf->state += n;
  return true;
}

bool protocol_append_str(ProtocolBuffer *pbuf, const char *key,
                         const char *val) {

  size_t size = (pbuf->buf + pbuf->len) - pbuf->state;
  int n;

  if (strchr(val, ' '))
    n = snprintf(pbuf->state, size, " %s:\"%s\"", key, val);
  else
    n = snprintf(pbuf->state, size, " %s:%s", key, val);

  if (n < 0 || (size_t)n >= size)
    return false;

  pbuf->state += n;
  return true;
}

bool protocol_append_ul(ProtocolBuffer *pbuf, const char *key,
                        unsigned long val) {

  size_t size = (pbuf->buf + pbuf->len) - pbuf->state;

  int n = snprintf(pbuf->state, size, " %s:%lu", key, val);

  if (n < 0 || (size_t)n >= size)
    return false;

  pbuf->state += n;
  return true;
}
