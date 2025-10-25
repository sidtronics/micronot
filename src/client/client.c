#include "../protocol.h"
#include <micronot/client.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int un_connect(const char *sock_path) {

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

NotificationID un_notify(int conn, char *text, NotificationType type,
                         u_int16_t mask, NotificationAttributes *attrs) {

  if (!text || !attrs->indicator)
    return false;

  char buf[256];
  ProtocolBuffer pbuf = ProtocolBufferInit(buf);

  ProtocolAppend(&pbuf, UNOT_CMD_NOTIFY, NULL);
  ProtocolAppend(&pbuf, UNOT_KEY_TEXT, text);

  switch (type) {
  case UNOT_TYPE_MESSAGE:
    ProtocolAppend(&pbuf, UNOT_KEY_TYPE, UNOT_VAL_MESSAGE);
    break;

  case UNOT_TYPE_SPINNER:
    ProtocolAppend(&pbuf, UNOT_KEY_TYPE, UNOT_VAL_SPINNER);
    break;
  }

  if (mask & UNIndicator) {
    ProtocolAppend(&pbuf, UNOT_KEY_INDICATOR, attrs->indicator);
  }

  else {
    ProtocolAppend(&pbuf, UNOT_KEY_INDICATOR_NAME, attrs->indicator);
  }

  if (mask & UNTextFG) {
    ProtocolAppendUL(&pbuf, UNOT_KEY_TEXT_FG, attrs->text_fg);
  }

  if (mask & UNIndicatorFG) {
    ProtocolAppendUL(&pbuf, UNOT_KEY_INDICATOR_FG, attrs->indicator_fg);
  }

  if (mask & UNTextFont) {
    ProtocolAppend(&pbuf, UNOT_KEY_TEXT_FONT, attrs->text_font);
  }

  if (mask & UNTimeout) {
    ProtocolAppendUL(&pbuf, UNOT_KEY_TIMEOUT, attrs->timeout);
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

// bool unot_return(int conn, int retval, NotificationID id) {
//
//   if (id == 0)
//     return false;
//
//   char buf[32];
//   sprintf(buf, "%s\n%s:%lu\n%s:%d\n\n", UNOT_CMD_RETURN, UNOT_KEY_NOTIF_ID,
//   id,
//           UNOT_KEY_RETURN_VALUE, retval != 0);
//
//   if (!protocol_send_block(conn, buf, sizeof(buf)))
//     return false;
//
//   if (!protocol_recv_block(conn, buf, sizeof(buf)))
//     return false;
//
//   char *status = strtok(buf, "\n");
//   if (strcmp(status, "OK") == 0)
//     return true;
//
//   return false;
// }

void un_disconnect(int conn) {
  if (conn >= 0) {
    if (close(conn) == -1) {
      perror("[unot:client] ERROR: close");
    }
  }
}
