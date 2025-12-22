#include "parser.h"
#include "utils.h"

bool parse_command(ProtocolBuffer *pbuf, Command *cmd) {

  assert(pbuf && "parse_command: pbuf");
  assert(cmd && "parse_command: cmd");

  cmd->mask = 0;
  const char *header;
  protocol_parse_header(pbuf, &header);

  if (*header == 0) {
    fprintf(stderr, "[unotd:server] ERROR: missing command\n");
    return false;
  }

#define X(a, b) else if (strcmp(header, b) == 0) cmd->kind = a;
  UNOT_COMMANDS
#undef X

  else {
    fprintf(stderr, "[unotd:server] ERROR: unknown command '%s'\n", header);
    return false;
  }

  const char *key;
  const char *val;
  while (protocol_parse_field(pbuf, &key, &val)) {

    if (!key)
      return false;

#define X(a, b, c)                                                             \
  else if (strcmp(key, b) == 0) {                                              \
    cmd->c = val;                                                              \
    cmd->mask |= a;                                                            \
  }
    UNOT_FIELDS_STR
#undef X

#define X(a, b, c)                                                             \
  else if (strcmp(key, b) == 0) {                                              \
    if (!utils_parse_ul(val, &cmd->c, 10)) {                                   \
      fprintf(stderr, "[unotd:server] ERROR: invalid numeric value: '%s'\n",   \
              val);                                                            \
      return false;                                                            \
    }                                                                          \
    cmd->mask |= a;                                                            \
  }
    UNOT_FIELDS_UL
#undef X

    else {
      fprintf(stderr, "[unotd:server] ERROR: unknown key: '%s'\n", key);
      return false;
    }
  }

  return true;
}
