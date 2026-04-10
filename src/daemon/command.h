#ifndef MICRONOT_COMMAND_H
#define MICRONOT_COMMAND_H

#include <stdint.h>

typedef struct _ServerCtx ServerCtx;
typedef struct _hf_message hf_message;

void command_handle(ServerCtx *sctx, uint64_t id, hf_message *msg);

#endif
