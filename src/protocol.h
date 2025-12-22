#ifndef MICRONOT_PROTOCOL_H
#define MICRONOT_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>

// // Commands
// #define UNOT_CMD_NOTIFY "NTF"
// #define UNOT_CMD_MODIFY "MDF"
//
// // Keys
// #define UNOT_KEY_TEXT "txt"
// #define UNOT_KEY_INDICATOR "ind"
// #define UNOT_KEY_INDICATOR_NAME "nme"
// #define UNOT_KEY_INDICATOR_FG "ifg"
// #define UNOT_KEY_TEXT_FG "tfg"
// #define UNOT_KEY_TEXT_FONT "tfn"
// #define UNOT_KEY_TIMEOUT "tim"
// #define UNOT_KEY_NOTIF_ID "nid"

#define UNOT_COMMANDS                                                          \
  X(UNOT_CMD_NOTIFY, "NTF")                                                    \
  X(UNOT_CMD_MODIFY, "MDF")

#define UNOT_FIELDS_STR                                                        \
  X(UNOT_KEY_TEXT, "txt", text)                                                \
  X(UNOT_KEY_INDICATOR, "ind", indicator)                                      \
  X(UNOT_KEY_INDICATOR_NAME, "nme", indicator_name)                            \
  X(UNOT_KEY_TEXT_FONT, "tfn", text_font)

#define UNOT_FIELDS_UL                                                         \
  X(UNOT_KEY_NOTIF_ID, "nid", notification_id)                                 \
  X(UNOT_KEY_INDICATOR_FG, "ifg", indicator_fg)                                \
  X(UNOT_KEY_TEXT_FG, "tfg", text_fg)                                          \
  X(UNOT_KEY_TIMEOUT, "tim", timeout)

typedef struct _ProtocolBuffer {
  char *buf;
  char *state;
  size_t len;
} ProtocolBuffer;

// Protocol parser
bool protocol_parse_header(ProtocolBuffer *pbuf, const char **cmd);
bool protocol_parse_field(ProtocolBuffer *pbuf, const char **key,
                          const char **val);

// Protocol builder functions
bool protocol_append_header(ProtocolBuffer *pbuf, char *cmd);
bool protocol_append_str(ProtocolBuffer *pbuf, const char *key,
                         const char *val);
bool protocol_append_ul(ProtocolBuffer *pbuf, const char *key,
                        unsigned long val);

#define protocol_append(pbuf, key, val)                                        \
  _Generic(((val) + 0),                                                        \
      char *: protocol_append_str,                                             \
      const char *: protocol_append_str,                                       \
      unsigned long: protocol_append_ul)((pbuf), (key), (val))

// Protocol I/O
bool protocol_send(int fd, ProtocolBuffer *pbuf);
bool protocol_recv(int fd, ProtocolBuffer *pbuf);

#endif
