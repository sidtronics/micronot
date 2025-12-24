#include "parser.h"
#include "utils.h"

static bool _validate_cmd_notify(Config *cfg, Command *cmd);

bool parse_command(ProtocolBuffer *pbuf, Config *cfg, Command *cmd) {

  assert(pbuf && "parse_command: pbuf");
  assert(cmd && "parse_command: cmd");
  assert(cfg && "parse_command: cfg");

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

  // Validation:
  switch (cmd->kind) {

  case UNOT_CMD_NOTIFY:
    _validate_cmd_notify(cfg, cmd);
    break;

  case UNOT_CMD_MODIFY:
    if (!IS_SET(cmd->mask, UNOT_KEY_NOTIF_ID))
      return false;
    break;

  default:
    assert(0 && "unreachable");
  }

  return true;
}

static bool _validate_cmd_notify(Config *cfg, Command *cmd) {

  if (!IS_SET(cmd->mask, UNOT_KEY_TEXT) ||
      !IS_SET(cmd->mask, UNOT_KEY_INDICATOR))
    return false;

  switch (indicator_classify(cmd->indicator)) {

  case INDICATOR_STR_RAW:
    cmd->custom_indicator = true;
    break;

  case INDICATOR_STR_NAME:
    if (!indicator_resolve_name(cfg, &cmd->indicator)) {
      fprintf(stderr,
              "[unotd:server] ERROR: indicator of name '%s' not found\n",
              cmd->indicator);
      return false;
    }
    cmd->custom_indicator = false;
    break;

  case INDICATOR_STR_INVALID:
    fprintf(stderr, "[unotd:server] ERROR: malformed indicator string\n");
    return false;

  default:
    assert(0 && "unreachable");
  }

  return true;
}
