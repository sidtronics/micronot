#ifndef MICRONOT_COMMAND_H
#define MICRONOT_COMMAND_H

#include <stdint.h>

typedef struct _ServerCtx ServerCtx;
typedef struct _hf_message hf_message;
typedef uint64_t ClientID;

void command_handle(ServerCtx *sctx, ClientID cid, hf_message *msg);

#endif
