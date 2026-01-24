#ifndef MICRONOT_COMMAND_H
#define MICRONOT_COMMAND_H

typedef struct _ServerCtx ServerCtx;
typedef struct _hf_message hf_message;

void command_handle(ServerCtx *sctx, hf_message *msg);

#endif
