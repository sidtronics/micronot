#include "../protocol.h"
#include <assert.h>
#include <micronot/client.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

typedef enum {
#define X(a, b, c) a = 1u << __COUNTER__,
  UNOT_FIELDS_STR UNOT_FIELDS_UL
#undef X
} KeyMask;

typedef struct {
#define X(a, b, c) const char *c;
  UNOT_FIELDS_STR
#undef X
#define X(a, b, c) unsigned long c;
  UNOT_FIELDS_UL
#undef X
  KeyMask mask;
} UnotAttrsImpl;

_Static_assert(sizeof(UnotAttrs) >= sizeof(UnotAttrsImpl),
               "UnotAttrs too small");

UnotConnection unot_connect(const char *sock_path) {

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

#define X(a, b, c)                                                             \
  void unot_attr_set_##c(UnotAttrs *attrs, const char *value) {                \
    UnotAttrsImpl *_attrs = (UnotAttrsImpl *)attrs;                            \
    _attrs->c = value;                                                         \
    _attrs->mask |= a;                                                         \
  }
UNOT_FIELDS_STR
#undef X

#define X(a, b, c)                                                             \
  void unot_attr_set_##c(UnotAttrs *attrs, unsigned long value) {              \
    UnotAttrsImpl *_attrs = (UnotAttrsImpl *)attrs;                            \
    _attrs->c = value;                                                         \
    _attrs->mask |= a;                                                         \
  }
UNOT_FIELDS_UL
#undef X

void unot_attr_reset(UnotAttrs *attrs) {

  UnotAttrsImpl *_attrs = (UnotAttrsImpl *)attrs;
  _attrs->mask = 0;
}

UnotID unot_notify(UnotConnection conn, UnotAttrs *attrs) {

  UnotAttrsImpl *_attrs = (UnotAttrsImpl *)attrs;

  if (!(_attrs->mask & UNOT_KEY_TEXT) ||
      !(_attrs->mask & (UNOT_KEY_INDICATOR | UNOT_KEY_INDICATOR_NAME)))
    return 0;

  char buf[256];
  ProtocolBuffer pbuf = {.buf = buf, .state = buf, .len = sizeof(buf)};
  protocol_append_header(&pbuf, "NTF");

#define X(a, b, c)                                                             \
  if (_attrs->mask & a)                                                        \
    protocol_append(&pbuf, b, _attrs->c);
  UNOT_FIELDS_STR
  UNOT_FIELDS_UL
#undef X

  if (!protocol_send(conn, &pbuf))
    return 0;

  if (!protocol_recv(conn, &pbuf))
    return 0;

  const char *key;
  const char *val;
  if (protocol_parse_header(&pbuf, &key) && strcmp(key, "OK") == 0) {
    if (protocol_parse_field(&pbuf, &key, &val) && strcmp(key, "nid") == 0) {
      return strtoul(val, NULL, 10);
    }
  }

  return 0;
}

// NotificationID unot_notify(int conn, char *text, u_int16_t mask,
//                            NotificationAttributes *attrs) {
//
//   if (!text || !attrs->indicator)
//     return false;
//
//   char buf[256];
//   ProtocolBuffer pbuf = {.buf = buf, .state = buf, .len = sizeof(buf)};
//
//   protocol_append_header(&pbuf, UNOT_CMD_NOTIFY);
//
//   protocol_append(&pbuf, UNOT_KEY_TEXT, text);
//
//   if (mask & UNIndicator) {
//     protocol_append(&pbuf, UNOT_KEY_INDICATOR, attrs->indicator);
//   }
//
//   else {
//     protocol_append(&pbuf, UNOT_KEY_INDICATOR_NAME, attrs->indicator);
//   }
//
//   if (mask & UNTextFG) {
//     protocol_append(&pbuf, UNOT_KEY_TEXT_FG, attrs->text_fg);
//   }
//
//   if (mask & UNIndicatorFG) {
//     protocol_append(&pbuf, UNOT_KEY_INDICATOR_FG, attrs->indicator_fg);
//   }
//
//   if (mask & UNTextFont) {
//     protocol_append(&pbuf, UNOT_KEY_TEXT_FONT, attrs->text_font);
//   }
//
//   if (mask & UNTimeout) {
//     protocol_append(&pbuf, UNOT_KEY_TIMEOUT, attrs->timeout);
//   }
//
//   if (!protocol_send(conn, &pbuf))
//     return 0;
//
//   if (!protocol_recv(conn, &pbuf))
//     return 0;
//
//   const char *key;
//   const char *val;
//   if (protocol_parse_header(&pbuf, &key) && strcmp(key, "OK") == 0) {
//     if (protocol_parse_field(&pbuf, &key, &val) &&
//         strcmp(key, UNOT_KEY_NOTIF_ID) == 0) {
//       return strtoul(val, NULL, 10);
//     }
//   }
//
//   return 0;
// }

void unot_disconnect(UnotConnection conn) {
  if (conn >= 0) {
    if (close(conn) == -1) {
      perror("[unot:client] ERROR: close");
    }
  }
}
