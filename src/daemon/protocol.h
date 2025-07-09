#ifndef UNOT_PROTOCOL_H
#define UNOT_PROTOCOL_H

#include "unotd.h"

int protocol_recv_command(int fd, char *buf, size_t len);
void protocol_handle_command(Unotd *unotd, int fd, char *buf, size_t len);

#endif
