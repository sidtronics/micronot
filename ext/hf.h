/*
 * HF — Header Fields v0.2.0
 *
 * Message Format
 * --------------
 *
 * A message consists of a header line followed by zero or more key–value pairs.
 * Lines are terminated by a newline character ('\n').
 *
 *     HEADER\n
 *     key1:val\n
 *     key2:val\n
 *     key3:val\n
 *     ...
 *     keyN:val\n
 *     \n
 *
 * An empty line marks the end of the message.
 *
 * Supported Value Types
 * ---------------------
 *
 *  String  (HF_STRING_T)
 *  Integer (HF_INTEGER_T)
 *  Double  (HF_DOUBLE_T)
 *  Bool    (HF_BOOL_T)
 *
 * Configuration Options
 * ---------------------
 *
 * HF_BUFFER_SIZE
 *     Size of the internal message buffer in bytes.
 *     Default: 512
 *
 * Application-Specific API Generation
 * -----------------------------------
 *
 * The library can generate application-specific APIs by defining the macros
 * HF_HEADERS and HF_FIELDS. These macros must be provided as X-macro lists with
 * the signatures described below.
 *
 * HF_FIELDS
 * ~~~~~~~~~
 *
 *     X(String, FlagIdentifier, Type, Name)
 *
 *     String
 *         Format-level key string.
 *
 *     FlagIdentifier
 *         Enumeration constant identifying the field. Used as a bit flag.
 *
 *     Type
 *         Data type of the field value.
 *
 *     Name
 *         Symbolic name used in the generated API, including:
 *             - Members of struct hf_message
 *             - Field getter and setter function names
 *
 * HF_HEADERS
 * ~~~~~~~~~~
 *
 *     X(String, Identifier, ValidFieldsMask)
 *
 *     String
 *         Format-level header string.
 *
 *     Identifier
 *         Enumeration constant identifying the header.
 *
 *     ValidFieldsMask
 *         Bitmask specifying which fields are valid for this header. Use flags
 *         defined in HF_FIELDS.
 */

#ifndef HF_H_
#define HF_H_

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>

typedef const char *HF_STRING_T;
typedef int64_t HF_INTEGER_T;
typedef double HF_DOUBLE_T;
typedef _Bool HF_BOOL_T;

#ifndef HF_BUFFER_SIZE
#define HF_BUFFER_SIZE 512
#endif

typedef enum {
  HF_ERROR_SUCCESS,
  HF_ERROR_IO_ERROR,
  HF_ERROR_IO_CLOSED,
  HF_ERROR_BUFFER_FULL,
  HF_ERROR_MSG_MALFORMED,
  HF_ERROR_MSG_INVALID_KEY,
  HF_ERROR_MSG_UNKNOWN_HEADER,
  HF_ERROR_MSG_CONVERSION_ERROR
} hf_error;

typedef struct _hf_context {
  uint32_t cur;
  uint32_t end;
  hf_error error;
  char buf[HF_BUFFER_SIZE];
} hf_context;

// IO
bool hf_message_recv_sync(int fd, hf_context *ctx);
bool hf_message_recv_async(int fd, hf_context *ctx);
bool hf_message_received(hf_context *ctx);

bool hf_message_send_sync(int fd, hf_context *ctx);
bool hf_message_send_async(int fd, hf_context *ctx);
bool hf_message_sent(hf_context *ctx);

bool hf_poll_sync(struct pollfd *fds, nfds_t nfds);

static inline void hf_clear_buffer(hf_context *ctx) {
  ctx->cur = 0;
  ctx->end = 0;
}

// Parsing
bool hf_parse_header(hf_context *ctx, const char **header);
bool hf_parse_field(hf_context *ctx, const char **key, const char **val);
bool hf_end_reached(hf_context *ctx);

// Building
bool hf_append_header(hf_context *ctx, const char *header);
bool hf_append_field_str(hf_context *ctx, const char *key, HF_STRING_T val);
bool hf_append_field_int(hf_context *ctx, const char *key, HF_INTEGER_T val);
bool hf_append_field_dbl(hf_context *ctx, const char *key, HF_DOUBLE_T val);
bool hf_append_field_bool(hf_context *ctx, const char *key, HF_BOOL_T val);

#define hf_append_field(ctx, key, val)                                         \
  _Generic((val),                                                              \
      HF_STRING_T: hf_append_field_str,                                        \
      char *: hf_append_field_str,                                             \
      HF_INTEGER_T: hf_append_field_int,                                       \
      int: hf_append_field_int,                                                \
      HF_DOUBLE_T: hf_append_field_dbl,                                        \
      HF_BOOL_T: hf_append_field_bool)((ctx), (key), (val))

bool hf_end_message(hf_context *ctx);

// Error
const char *hf_get_error_string(hf_context *ctx);

static inline hf_error hf_get_error_code(hf_context *ctx) { return ctx->error; }

static inline void hf_clear_error(hf_context *ctx) {
  ctx->error = HF_ERROR_SUCCESS;
}

#if defined(HF_HEADERS) && defined(HF_FIELDS)

// Generate key flags
enum __hf_key_id {
#define X(String, FlagIdentifier, Type, Name) __##FlagIdentifier,
  HF_FIELDS
#undef X
};

enum __hf_key_flags {
#define X(String, FlagIdentifier, Type, Name)                                  \
  FlagIdentifier = 1u << __##FlagIdentifier,
  HF_FIELDS
#undef X
};

typedef enum {
#define X(String, Identifier, ValidFieldsMask) Identifier,
  HF_HEADERS
#undef X
} hf_header;

typedef struct _hf_message {
  hf_header _header;
  uint16_t _mask;
#define X(String, FlagIdentifier, Type, Name) Type Name;
  HF_FIELDS
#undef X
} hf_message;

bool hf_message_parse(hf_context *ctx, hf_message *msg);
bool hf_message_build(hf_context *ctx, hf_message *msg);

static inline void hf_message_set_header(hf_message *msg, hf_header header) {
  msg->_header = header;
}

static inline hf_header hf_message_get_header(hf_message *msg) {
  return msg->_header;
}

#define X(String, FlagIdentifier, Type, Name)                                  \
                                                                               \
  static inline Type hf_message_get_field_##Name(hf_message *msg) {            \
    return msg->Name;                                                          \
  }                                                                            \
                                                                               \
  static inline void hf_message_set_field_##Name(hf_message *msg, Type Name) { \
    msg->Name = Name;                                                          \
    msg->_mask |= FlagIdentifier;                                              \
  }                                                                            \
                                                                               \
  static inline void hf_message_clear_field_##Name(hf_message *msg) {          \
    msg->_mask &= ~FlagIdentifier;                                             \
  }                                                                            \
                                                                               \
  static inline bool hf_message_has_field_##Name(hf_message *msg) {            \
    return msg->_mask & FlagIdentifier;                                        \
  }

HF_FIELDS
#undef X

static inline void hf_message_clear_fields(hf_message *msg) { msg->_mask = 0; }

static inline bool hf_message_mask_has_all(hf_message *msg, uint16_t mask) {
  return (msg->_mask & mask) == mask;
}

static inline bool hf_message_mask_has_any(hf_message *msg, uint16_t mask) {
  return (msg->_mask & mask);
}

#endif // defined(HF_HEADERS) && defined(HF_FIELDS)

#if defined(HF_IMPLEMENTATION)

bool hf_poll_sync(struct pollfd *fds, nfds_t nfds) {
  int c = poll(fds, nfds, -1);
  return c > 0;
}

bool hf_message_recv_async(int fd, hf_context *ctx) {

  assert(ctx->error == HF_ERROR_SUCCESS);

  size_t room = sizeof(ctx->buf) - ctx->end;
  if (room == 0) {
    ctx->error = HF_ERROR_BUFFER_FULL;
    return false;
  }

  ssize_t n = recv(fd, ctx->buf + ctx->end, room, MSG_DONTWAIT);

  if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return true;
    ctx->error = HF_ERROR_IO_ERROR;
    return false;
  }

  if (n == 0) {
    ctx->error = HF_ERROR_IO_CLOSED;
    return false;
  }

  ctx->end += n;
  return true;
}

bool hf_message_send_async(int fd, hf_context *ctx) {

  assert(ctx->error == HF_ERROR_SUCCESS);

  size_t size = ctx->end - ctx->cur;
  ssize_t n = send(fd, ctx->buf + ctx->cur, size, MSG_DONTWAIT);

  if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return true;
    ctx->error = HF_ERROR_IO_ERROR;
    return false;
  }

  ctx->cur += n;
  return true;
}

bool hf_message_received(hf_context *ctx) {

  for (; ctx->cur + 1 < ctx->end; ++ctx->cur) {
    if (memcmp(ctx->buf + ctx->cur, "\n\n", 2) == 0)
      return true;
  }

  return false;
}

bool hf_message_recv_sync(int fd, hf_context *ctx) {

  hf_clear_buffer(ctx);

  struct pollfd pfd = {.fd = fd, .events = POLLIN};

  do {
    if (!hf_poll_sync(&pfd, 1)) {
      ctx->error = HF_ERROR_IO_ERROR;
      return false;
    }
    if (!hf_message_recv_async(fd, ctx))
      return false;
  } while (!hf_message_received(ctx));

  return true;
}

bool hf_message_sent(hf_context *ctx) { return ctx->cur == ctx->end; }

bool hf_message_send_sync(int fd, hf_context *ctx) {

  ctx->cur = 0;

  struct pollfd pfd = {.fd = fd, .events = POLLOUT};

  do {
    if (!hf_poll_sync(&pfd, 1)) {
      ctx->error = HF_ERROR_IO_ERROR;
      return false;
    }
    if (!hf_message_send_async(fd, ctx))
      return false;
  } while (!hf_message_sent(ctx));

  return true;
}

static char *__hf_parse_line(hf_context *ctx) {

  assert(ctx->end > ctx->cur);

  char *start = ctx->buf + ctx->cur;
  char *end = (char *)memchr(start, '\n', ctx->end - ctx->cur);
  if (!end)
    return NULL;

  *end = '\0';
  ctx->cur += (end - start) + 1;
  return start;
}

bool hf_parse_header(hf_context *ctx, const char **header) {

  assert(header);
  assert(ctx->error == HF_ERROR_SUCCESS);

  ctx->cur = 0;
  *header = __hf_parse_line(ctx);
  if (!*header || !**header) {
    ctx->error = HF_ERROR_MSG_MALFORMED;
    return false;
  }

  return true;
}

bool hf_parse_field(hf_context *ctx, const char **key, const char **val) {

  assert(key);
  assert(val);
  assert(ctx->error == HF_ERROR_SUCCESS);

  char *line = __hf_parse_line(ctx);
  if (!line) {
    ctx->error = HF_ERROR_MSG_MALFORMED;
    return false;
  }

  *key = line;
  char *colon = strchr(line, ':');

  /* check missing colon, key or value */
  if (!colon || *key == colon || !colon[1]) {
    ctx->error = HF_ERROR_MSG_MALFORMED;
    return false;
  }

  *colon = '\0';
  *val = colon + 1;
  return true;
}

bool hf_end_reached(hf_context *ctx) {
  return ctx->cur < ctx->end && ctx->buf[ctx->cur] == '\n';
}

static bool __hf_append(hf_context *ctx, const char *fmt, ...) {

  size_t room = sizeof(ctx->buf) - ctx->end;

  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(ctx->buf + ctx->end, room, fmt, ap);
  va_end(ap);

  if (n < 0 || (size_t)n > room) {
    ctx->error = HF_ERROR_BUFFER_FULL;
    return false;
  }

  ctx->end += n;
  return true;
}

bool hf_append_header(hf_context *ctx, const char *header) {

  assert(ctx->error == HF_ERROR_SUCCESS);
  hf_clear_buffer(ctx);
  return __hf_append(ctx, "%s\n", header);
}

bool hf_append_field_str(hf_context *ctx, const char *key, HF_STRING_T val) {

  assert(ctx->error == HF_ERROR_SUCCESS);
  return __hf_append(ctx, "%s:%s\n", key, val);
}

bool hf_append_field_int(hf_context *ctx, const char *key, HF_INTEGER_T val) {

  assert(ctx->error == HF_ERROR_SUCCESS);
  return __hf_append(ctx, "%s:%" PRId64 "\n", key, val);
}

bool hf_append_field_dbl(hf_context *ctx, const char *key, HF_DOUBLE_T val) {

  assert(ctx->error == HF_ERROR_SUCCESS);
  return __hf_append(ctx, "%s:%f\n", key, val);
}

bool hf_append_field_bool(hf_context *ctx, const char *key, HF_BOOL_T val) {

  assert(ctx->error == HF_ERROR_SUCCESS);
  return __hf_append(ctx, "%s:%s\n", key, val ? "true" : "false");
}

bool hf_end_message(hf_context *ctx) {

  assert(ctx->error == HF_ERROR_SUCCESS);
  return __hf_append(ctx, "\n");
}

const char *hf_get_error_string(hf_context *ctx) {

  switch (ctx->error) {
  case HF_ERROR_SUCCESS:
    return "success";
  case HF_ERROR_IO_ERROR:
    return "I/O error during communication";
  case HF_ERROR_IO_CLOSED:
    return "connection closed by peer";
  case HF_ERROR_BUFFER_FULL:
    return "buffer capacity exceeded";
  case HF_ERROR_MSG_MALFORMED:
    return "malformed protocol message received";
  case HF_ERROR_MSG_INVALID_KEY:
    return "invalid key for the current header";
  case HF_ERROR_MSG_UNKNOWN_HEADER:
    return "unknown header encountered";
  case HF_ERROR_MSG_CONVERSION_ERROR:
    return "type conversion error";
  }

  assert(0 && "unreachable");
  return NULL;
}

#if defined(HF_HEADERS) && defined(HF_FIELDS)

static inline bool __hf_tostr(HF_STRING_T *dest, const char *src) {

  *dest = src;
  return true;
}

static inline bool __hf_toint(HF_INTEGER_T *dest, const char *src) {

  char *endptr = NULL;
  long long val;

  errno = 0;
  val = strtoll(src, &endptr, 10);

  if (errno == ERANGE || *endptr != '\0')
    return false;

  if (val < INT64_MIN || val > INT64_MAX)
    return false;

  *dest = (HF_INTEGER_T)val;
  return true;
}

static inline bool __hf_todbl(HF_DOUBLE_T *dest, const char *src) {

  char *endptr = NULL;
  double val;

  errno = 0;
  val = strtod(src, &endptr);

  if (errno == ERANGE || *endptr != '\0')
    return false;

  *dest = val;
  return true;
}

static inline bool __hf_tobool(HF_BOOL_T *dest, const char *src) {

  if (strcmp(src, "true") == 0) {
    *dest = true;
    return true;
  }

  if (strcmp(src, "false") == 0) {
    *dest = false;
    return true;
  }

  return false;
}

bool hf_message_parse(hf_context *ctx, hf_message *msg) {

  assert(ctx->error == HF_ERROR_SUCCESS);

  msg->_mask = 0;
  const char *header;
  if (!hf_parse_header(ctx, &header))
    return false;

  uint16_t vmask = 0;

  if (0) {
  }
#define X(String, Identifier, ValidFieldsMask)                                 \
  else if (strcmp(String, header) == 0) {                                      \
    msg->_header = Identifier;                                                 \
    vmask = ValidFieldsMask;                                                   \
  }
  HF_HEADERS
#undef X
  else {
    ctx->error = HF_ERROR_MSG_UNKNOWN_HEADER;
    return false;
  }

  const char *key, *val;
  while (!hf_end_reached(ctx)) {

    if (!hf_parse_field(ctx, &key, &val))
      return false;

    if (0) {
    }
#define X(String, FlagIdentifier, Type, Name)                                  \
  else if ((vmask & FlagIdentifier) && strcmp(key, String) == 0) {             \
    if (!(_Generic((msg->Name),                                                \
              HF_STRING_T: __hf_tostr,                                         \
              HF_INTEGER_T: __hf_toint,                                        \
              HF_DOUBLE_T: __hf_todbl,                                         \
              HF_BOOL_T: __hf_tobool))(&msg->Name, val)) {                     \
      ctx->error = HF_ERROR_MSG_CONVERSION_ERROR;                              \
      return false;                                                            \
    }                                                                          \
    msg->_mask |= FlagIdentifier;                                              \
  }
    HF_FIELDS
#undef X
    else {
      ctx->error = HF_ERROR_MSG_INVALID_KEY;
      return false;
    }
  }

  return true;
}

bool hf_message_build(hf_context *ctx, hf_message *msg) {

  assert(ctx->error == HF_ERROR_SUCCESS);

  uint16_t mask = 0;

  switch (msg->_header) {
#define X(String, Identifier, ValidFieldsMask)                                 \
  case Identifier:                                                             \
    if (!hf_append_header(ctx, String))                                        \
      return false;                                                            \
    mask = ValidFieldsMask;                                                    \
    break;
    HF_HEADERS
#undef X
  default:
    assert(0 && "unreachable");
  }

#define X(String, FlagIdentifier, Type, Name)                                  \
  if (FlagIdentifier & mask & msg->_mask) {                                    \
    if (!hf_append_field(ctx, String, msg->Name))                              \
      return false;                                                            \
  }
  HF_FIELDS
#undef X

  if (!hf_end_message(ctx))
    return false;

  return true;
}

#endif // defined(HF_HEADERS) && defined(HF_FIELDS)
#endif // HF_IMPLEMENTATION
#endif // HF_H_

/*  Revision History:
 *
 *   0.1 (2026-01-20) Initial release
 *   0.2.0 (2026-05-01) Add support for boolean type HF_BOOL_T
 *
 */

/*
 * Copyright 2026 Siddhesh Dharme <siddheshdharme18@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
