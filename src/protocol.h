#ifndef MICRONOT_PROTOCOL_H
#define MICRONOT_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>

// Commands
#define UNOT_CMD_NOTIFY "NTF"
#define UNOT_CMD_MODIFY "MDF"

// Keys
#define UNOT_KEY_TEXT "txt"
#define UNOT_KEY_INDICATOR "ind"
#define UNOT_KEY_INDICATOR_NAME "nme"
#define UNOT_KEY_INDICATOR_FG "ifg"
#define UNOT_KEY_TEXT_FG "tfg"
#define UNOT_KEY_TEXT_FONT "tfn"
#define UNOT_KEY_TIMEOUT "tim"
#define UNOT_KEY_NOTIF_ID "nid"

typedef struct _ProtocolPair {
  char *key;
  union {
    char *val;
    unsigned long ul_val;
  };
} ProtocolPair;

typedef struct _ProtocolBuffer {
  char *buf;
  char *state;
  size_t len;
} ProtocolBuffer;

bool protocol_send(int fd, ProtocolBuffer *pbuf);
bool protocol_recv(int fd, ProtocolBuffer *pbuf);
bool protocol_parse(ProtocolBuffer *pbuf, ProtocolPair *pair);
bool protocol_append(ProtocolBuffer *pbuf, ProtocolPair pair, bool ul);

#define ProtocolBufferInit(buf)                                                \
  {.buf = (buf), .state = (buf), .len = sizeof(buf)}

#define ProtocolAppend(pbuf, k, v)                                             \
  (protocol_append((pbuf), (ProtocolPair){.key = (k), .val = (v)}, false))

#define ProtocolAppendUL(pbuf, k, v)                                           \
  (protocol_append((pbuf), (ProtocolPair){.key = (k), .ul_val = (v)}, true))

#endif
