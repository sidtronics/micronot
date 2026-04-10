#ifndef MICRONOT_CLIENT_H
#define MICRONOT_CLIENT_H
#include <stdbool.h>
#include <sys/types.h>

typedef struct {
  u_int64_t _opaque[9];
} UnotAttrs;

typedef int UnotConnection;
typedef unsigned long UnotID;

UnotConnection unot_connect(const char *sock_path);
void unot_disconnect(UnotConnection conn);

void unot_attr_set_text(UnotAttrs *attrs, const char *value);
void unot_attr_set_indicator(UnotAttrs *attrs, const char *value);
void unot_attr_set_text_font(UnotAttrs *attrs, const char *value);
void unot_attr_set_text_fg(UnotAttrs *attrs, int64_t value);
void unot_attr_set_indicator_fg(UnotAttrs *attrs, int64_t value);
void unot_attr_set_persistent(UnotAttrs *attrs, int64_t value);
void unot_attr_set_timeout(UnotAttrs *attrs, int64_t value);

void unot_attr_clear_text(UnotAttrs *attrs);
void unot_attr_clear_indicator(UnotAttrs *attrs);
void unot_attr_clear_text_font(UnotAttrs *attrs);
void unot_attr_clear_text_fg(UnotAttrs *attrs);
void unot_attr_clear_indicator_fg(UnotAttrs *attrs);
void unot_attr_clear_persistent(UnotAttrs *attrs);
void unot_attr_clear_timeout(UnotAttrs *attrs);

void unot_attr_clear(UnotAttrs *attrs);

bool unot_debug(UnotConnection conn);

UnotID unot_notify(UnotConnection conn, UnotAttrs *attrs);
UnotID unot_modify(UnotConnection conn, UnotAttrs *attrs, UnotID id);
UnotID unot_close(UnotConnection conn, UnotID id);

#endif
