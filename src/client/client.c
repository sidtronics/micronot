#include <assert.h>
#include <micronot/client.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>

#include "../hf_defs.h"
#define HF_IMPLEMENTATION
#include <hf.h>

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

UnotConnection unot_connect(const char *host, uint16_t port) {

  if (host == NULL) {
    host = UNOT_DEFAULT_SOCKET;
  }

  // Unix socket
  if (host[0] == '/') {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
      perror("[unot:client] ERROR: socket");
      return -1;
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, host, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
      perror("[unot:client] ERROR: connect (unix)");
      close(fd);
      return -1;
    }
    return fd;
  }

  // TCP socket
  char port_str[6];
  snprintf(port_str, sizeof(port_str), "%u", port);

  struct addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo *res = NULL;
  int err = getaddrinfo(host, port_str, &hints, &res);
  if (err != 0) {
    fprintf(stderr, "[unot:client] ERROR: getaddrinfo: %s\n",
            gai_strerror(err));
    return -1;
  }

  int fd = -1;
  for (struct addrinfo *rp = res; rp != NULL; rp = rp->ai_next) {
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd < 0)
      continue;

    if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) // success
      break;

    close(fd);
    fd = -1;
  }

  freeaddrinfo(res);

  if (fd < 0) {
    perror("[unot:client] ERROR: connect (tcp)");
    fprintf(stderr, "[unot:client] ERROR: could not connect to %s:%u\n", host,
            port);
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

  if (!hf_message_mask_has_all(msg, UNOT_F_TEXT)) {
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
