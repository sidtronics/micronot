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
  } while ((recv_end = memchr(recv_beg, '\n', n)) == NULL);

  *recv_end = 0;
  pbuf->state = pbuf->buf;
  return true;
}

bool protocol_send(int fd, ProtocolBuffer *pbuf) {

  size_t bytes = strnlen(pbuf->buf, pbuf->len - 1);
  pbuf->buf[bytes] = '\n';
  bytes++;

  size_t bytes_sent = 0;
  while (bytes_sent < bytes) {

    ssize_t n = send(fd, pbuf->buf + bytes_sent, bytes - bytes_sent, 0);

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

bool protocol_parse(ProtocolBuffer *pbuf, ProtocolPair *pair) {

  // TODO: handle '\' as escaping character to allow use of double quotes.
  // TODO: handle empty keys and values.

  if (pbuf->state == pbuf->buf) {

    pbuf->state = strchr(pbuf->buf, ' ');

    if (pbuf->state)
      *pbuf->state++ = '\0';

    *pair = (ProtocolPair){pbuf->buf, NULL};
    return true;
  }

  if (!pbuf->state)
    return false;

  pair->key = pbuf->state;
  pbuf->state = strchr(pbuf->state, ':');
  if (!pbuf->state) {
    fprintf(stderr, "[protocol] ERROR: malformed pair, expected ':'\n");
    *pair = (ProtocolPair){NULL, NULL};
    return true;
  }

  *pbuf->state++ = '\0';

  if (*pbuf->state == '"') {

    pair->val = ++pbuf->state;
    pbuf->state = strchr(pbuf->state, '"');
    if (!pbuf->state) {
      fprintf(stderr, "[protocol] ERROR: missing closing '\"''\n");
      *pair = (ProtocolPair){NULL, NULL};
      return true;
    }

    *pbuf->state++ = '\0';
  }

  else
    pair->val = pbuf->state;

  pbuf->state = strchr(pbuf->state, ' ');
  if (pbuf->state)
    *pbuf->state++ = '\0';

  return true;
}

bool protocol_append(ProtocolBuffer *pbuf, ProtocolPair pair, bool ul) {

  size_t size = (pbuf->buf + pbuf->len) - pbuf->state;
  int n;

  if (pbuf->state == pbuf->buf)
    n = snprintf(pbuf->state, size, "%s", pair.key);

  else {
    if (ul)
      n = snprintf(pbuf->state, size, " %s:%lu", pair.key, pair.ul_val);
    else {
      if (strchr(pair.val, ' '))
        n = snprintf(pbuf->state, size, " %s:\"%s\"", pair.key, pair.val);
      else
        n = snprintf(pbuf->state, size, " %s:%s", pair.key, pair.val);
    }
  }

  if (n < 0 || (size_t)n >= size)
    return false;

  pbuf->state += n;
  return true;
}
