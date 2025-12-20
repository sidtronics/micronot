#include "../protocol.h"
#include <micronot/client.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int unot_connect(const char *sock_path) {

  int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("[unot:client] ERROR: socket");
    return -1;
  }

  struct sockaddr_un addr = {0};
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);

  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) ==
      -1) {
    perror("[unot:client] ERROR: connect");
    close(sockfd);
    return -1;
  }

  return sockfd;
}

NotificationID unot_notify(int conn, char *text, u_int16_t mask,
                           NotificationAttributes *attrs) {

  if (!text || !attrs->indicator)
    return false;

  char buf[256];
  ProtocolBuffer pbuf = {.buf = buf, .state = buf, .len = sizeof(buf)};

  protocol_begin(&pbuf, UNOT_CMD_NOTIFY);

  protocol_append(&pbuf, UNOT_KEY_TEXT, text);

  if (mask & UNIndicator) {
    protocol_append(&pbuf, UNOT_KEY_INDICATOR, attrs->indicator);
  }

  else {
    protocol_append(&pbuf, UNOT_KEY_INDICATOR_NAME, attrs->indicator);
  }

  if (mask & UNTextFG) {
    protocol_append(&pbuf, UNOT_KEY_TEXT_FG, attrs->text_fg);
  }

  if (mask & UNIndicatorFG) {
    protocol_append(&pbuf, UNOT_KEY_INDICATOR_FG, attrs->indicator_fg);
  }

  if (mask & UNTextFont) {
    protocol_append(&pbuf, UNOT_KEY_TEXT_FONT, attrs->text_font);
  }

  if (mask & UNTimeout) {
    protocol_append(&pbuf, UNOT_KEY_TIMEOUT, attrs->timeout);
  }

  if (!protocol_send(conn, &pbuf))
    return 0;

  if (!protocol_recv(conn, &pbuf))
    return 0;

  ProtocolPair pair;
  if (protocol_parse(&pbuf, &pair) && strcmp(pair.key, "OK") == 0) {
    if (protocol_parse(&pbuf, &pair) &&
        strcmp(pair.key, UNOT_KEY_NOTIF_ID) == 0) {
      return strtoul(pair.val, NULL, 10);
    }
  }

  return 0;
}

void unot_disconnect(int conn) {
  if (conn >= 0) {
    if (close(conn) == -1) {
      perror("[unot:client] ERROR: close");
    }
  }
}
