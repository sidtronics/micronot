#include "../protocol.h"
#include <micronot/client.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int unot_get_connection(const char *sock_path) {

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

bool unot_notify(int conn, const char *text, NotificationType type,
                 u_int16_t mask, NotificationAttributes *attrs,
                 NotificationID *id) {

  if (!text || (!attrs->indicator && !attrs->indicator_name))
    return false;

  if (type == UNOT_TYPE_SPINNER && !id)
    return false;

  size_t size = 256;
  char buf[size];
  size_t offset = 0;

  offset += snprintf(buf + offset, size - offset, "%s\n", UNOT_CMD_NOTIFY);
  offset +=
      snprintf(buf + offset, size - offset, "%s:%s\n", UNOT_KEY_TEXT, text);

  if (type == UNOT_TYPE_MESSAGE) {
    offset += snprintf(buf + offset, size - offset, "%s:%s\n", UNOT_KEY_TYPE,
                       UNOT_VAL_MESSAGE);
  }

  else if (type == UNOT_TYPE_SPINNER) {
    offset += snprintf(buf + offset, size - offset, "%s:%s\n", UNOT_KEY_TYPE,
                       UNOT_VAL_SPINNER);
  }

  if (mask & UNIndicator) {
    offset += snprintf(buf + offset, size - offset, "%s:%s\n",
                       UNOT_KEY_INDICATOR, attrs->indicator);
  }

  else {
    offset += snprintf(buf + offset, size - offset, "%s:%s\n",
                       UNOT_KEY_INDICATOR_NAME, attrs->indicator_name);
  }

  if (mask & UNTextFG) {
    offset += snprintf(buf + offset, size - offset, "%s:%lx\n",
                       UNOT_KEY_TEXT_FG, attrs->text_fg);
  }

  if (mask & UNIndicatorFG) {
    offset += snprintf(buf + offset, size - offset, "%s:%lx\n",
                       UNOT_KEY_INDICATOR_FG, attrs->indicator_fg);
  }

  if (mask & UNTextFont) {
    offset += snprintf(buf + offset, size - offset, "%s:%s\n",
                       UNOT_KEY_TEXT_FONT, attrs->text_font);
  }

  if (mask & UNTimeout) {
    offset += snprintf(buf + offset, size - offset, "%s:%lu\n",
                       UNOT_KEY_TIMEOUT, attrs->timeout);
  }

  offset += snprintf(buf + offset, size - offset, "\n");

  if (!protocol_send_block(conn, buf, size))
    return false;

  if (!protocol_recv_block(conn, buf, size))
    return false;

  char *status = strtok(buf, "\n");

  if (strcmp(status, "OK") == 0) {
    char *field = strtok(NULL, ":");
    if (field != NULL) {
      if (strcmp(field, UNOT_KEY_NOTIF_ID) == 0) {
        char *id_str = strtok(NULL, "\n");
        *id = strtoul(id_str, NULL, 10);
      } else
        return false;
    }
    return true;
  }
  return false;
}

bool unot_return(int conn, int retval, NotificationID id) {

  if (id == 0)
    return false;

  char buf[32];
  sprintf(buf, "%s\n%s:%lu\n%s:%d\n\n", UNOT_CMD_RETURN, UNOT_KEY_NOTIF_ID, id,
          UNOT_KEY_RETURN_VALUE, retval != 0);

  if (!protocol_send_block(conn, buf, sizeof(buf)))
    return false;

  if (!protocol_recv_block(conn, buf, sizeof(buf)))
    return false;

  char *status = strtok(buf, "\n");
  if (strcmp(status, "OK") == 0)
    return true;

  return false;
}

void unot_close_connection(int conn) {
  if (conn >= 0) {
    if (close(conn) == -1) {
      perror("[unot:client] ERROR: close");
    }
  }
}
