#ifndef MICRONOT_PROTOCOL_H
#define MICRONOT_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>

// Commands
#define UNOT_CMD_NOTIFY "NTF"
#define UNOT_CMD_RETURN "RET"

// Keys
#define UNOT_KEY_TEXT "txt"
#define UNOT_KEY_INDICATOR "ind"
#define UNOT_KEY_INDICATOR_NAME "nme"
#define UNOT_KEY_INDICATOR_FG "ifg"
#define UNOT_KEY_TEXT_FG "tfg"
#define UNOT_KEY_TEXT_FONT "tfn"
#define UNOT_KEY_TIMEOUT "tim"
#define UNOT_KEY_TYPE "typ"
#define UNOT_KEY_NOTIF_ID "nid"
#define UNOT_KEY_RETURN_VALUE "ret"

// Values (for UNOT_KEY_TYPE)
#define UNOT_VAL_MESSAGE "msg"
#define UNOT_VAL_SPINNER "spn"

bool protocol_send_block(int fd, char *buf, size_t len);
bool protocol_recv_block(int fd, char *buf, size_t len);

#endif
