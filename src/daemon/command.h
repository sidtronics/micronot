#ifndef MICRONOT_COMMAND_H
#define MICRONOT_COMMAND_H

#include "unotd.h"
#include "../protocol.h"

void command_handle(Unotd *unotd, int fd, ProtocolBuffer *pbuf);

#endif
