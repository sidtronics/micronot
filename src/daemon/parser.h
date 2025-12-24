#ifndef MICRONOT_PARSER_H
#define MICRONOT_PARSER_H

#include "../protocol.h"
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>

typedef enum : u_int16_t {
#define X(a, b) a,
  UNOT_COMMANDS
#undef X
} CommandKind;

typedef enum : u_int16_t {
#define X(a, b, c) a = 1u << __COUNTER__,
  UNOT_FIELDS_STR UNOT_FIELDS_UL
#undef X
} KeyMask;

typedef struct {
  CommandKind kind;
  KeyMask mask;
  bool custom_indicator;

#define X(a, b, c) const char *c;
  UNOT_FIELDS_STR
#undef X

#define X(a, b, c) unsigned long c;
  UNOT_FIELDS_UL
#undef X
} Command;

#define IS_SET(mask, flag) (((mask) & (flag)) != 0)

typedef struct _Config Config;

bool parse_command(ProtocolBuffer *pbuf, Config *cfg, Command *cmd);

#endif
