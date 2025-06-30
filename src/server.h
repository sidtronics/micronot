#ifndef UNOT_SERVER_H
#define UNOT_SERVER_H

#include "unotd.h"

#define UNOT_MIN_PFDS 10
#define UNOT_SOCK_PATH "/tmp/unotd.sock"

void server_init(Unotd *unotd);
void server_process_connections(Unotd *unotd);



#endif
