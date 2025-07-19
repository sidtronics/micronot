#ifndef MICRONOT_COMMAND_H
#define MICRONOT_COMMAND_H

#include "unotd.h"

void command_handle(Unotd *unotd, int fd, char *buf, size_t len);

#endif
