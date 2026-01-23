#ifndef MICRONOT_COMMAND_H
#define MICRONOT_COMMAND_H

#include "../protocol.h"

typedef struct _ServerCtx ServerCtx;
void command_handle(ServerCtx *sctx, int fd, ProtocolBuffer *pbuf);

#endif
