#include <assert.h>
#include <micronot/client.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "../hf_defs.h"
#define HF_IMPLEMENTATION
#include "../hf.h"

_Static_assert(sizeof(UnotAttrs) >= sizeof(hf_message),
               "UnotAttrs not big enough");

#define X(String, FlagIdentifier, Type, Name)                                  \
  void unot_attr_set_##Name(UnotAttrs *attrs, Type value) {                    \
    hf_message_set_field_##Name((hf_message *)attrs, value);                   \
  }                                                                            \
                                                                               \
  void unot_attr_clear_##Name(UnotAttrs *attrs) {                              \
    hf_message_clear_field_##Name((hf_message *)attrs);                        \
  }
HF_FIELDS
#undef X

void unot_attr_clear(UnotAttrs *attrs) {
  hf_message_clear_fields((hf_message *)attrs);
}

UnotConnection unot_connect(const char *sock_path) {

  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("[unot:client] ERROR: socket");
    return -1;
  }

  struct sockaddr_un addr = {0};
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);

  if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
    perror("[unot:client] ERROR: connect");
    close(fd);
    return -1;
  }

  return fd;
}

bool _hf_message_send_recv_sync(UnotConnection conn, hf_message *s,
                                hf_message *r) {

  hf_context ctx = {0};

  if (!hf_message_build(&ctx, s)) {
    fprintf(stderr, "[libunotclient]: %s\n", hf_get_error_string(&ctx));
    return 0;
  }

  if (!hf_message_send_sync(conn, &ctx)) {
    fprintf(stderr, "[libunotclient]: %s\n", hf_get_error_string(&ctx));
    return 0;
  }

  if (!hf_message_recv_sync(conn, &ctx)) {
    fprintf(stderr, "[libunotclient]: %s\n", hf_get_error_string(&ctx));
    return 0;
  }

  if (!hf_message_parse(&ctx, r)) {
    fprintf(stderr, "[libunotclient]: %s\n", hf_get_error_string(&ctx));
    return 0;
  }

  return 1;
}

UnotID unot_notify(UnotConnection conn, UnotAttrs *attrs) {

  hf_message *msg = (hf_message *)attrs;
  hf_message res = {0};

  if (!hf_message_mask_has_all(msg, UNOT_F_TEXT | UNOT_F_INDICATOR)) {
    return 0;
  }

  hf_message_set_header(msg, UNOT_H_NOTIFY);

  if (!_hf_message_send_recv_sync(conn, msg, &res))
    return 0;

  if (hf_message_get_header(&res) == UNOT_H_OK) {
    if (hf_message_has_field_id(&res)) {
      return hf_message_get_field_id(&res);
    }
  }

  return 0;
}

bool unot_modify(UnotConnection conn, UnotAttrs *attrs, UnotID id) {

  hf_message *msg = (hf_message *)attrs;
  hf_message res = {0};

  hf_message_set_header(msg, UNOT_H_MODIFY);
  hf_message_set_field_id(msg, id);

  if (!_hf_message_send_recv_sync(conn, msg, &res))
    return 0;

  return hf_message_get_header(&res) == UNOT_H_OK;
}

bool unot_debug(UnotConnection conn) {

  hf_message msg = {0};

  hf_message_set_header(&msg, UNOT_H_DEBUG);

  if (!_hf_message_send_recv_sync(conn, &msg, &msg))
    return 0;

  return hf_message_get_header(&msg) == UNOT_H_OK;
}

void unot_disconnect(UnotConnection conn) {
  if (conn >= 0) {
    if (close(conn) == -1) {
      perror("[unot:client] ERROR: close");
    }
  }
}
