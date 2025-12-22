#ifndef MICRONOT_CLIENT_H
#define MICRONOT_CLIENT_H
#include <stdbool.h>
#include <sys/types.h>

// #define UNIndicator (1 << 0)
// #define UNIndicatorName (1 << 1)
// #define UNTextFG (1 << 2)
// #define UNIndicatorFG (1 << 3)
// #define UNTextFont (1 << 4)
// #define UNTimeout (1 << 5)
//
// typedef struct {
//   char *indicator;
//   unsigned long text_fg;
//   unsigned long indicator_fg;
//   char *text_font;
//   unsigned long timeout;
// } NotificationAttributes;

// void unot_attr_unset_indicator(UnotAttrs *attrs);
// void unot_attr_unset_text_font(UnotAttrs *attrs);
// void unot_attr_unset_text_fg(UnotAttrs *attrs);
// void unot_attr_unset_indicator_fg(UnotAttrs *attrs);
// void unot_attr_unset_timeout(UnotAttrs *attrs);

// typedef unsigned long NotificationID;

// NotificationID unot_notify(int conn, char *text, u_int16_t mask,
// NotificationAttributes *attrs);

typedef struct {
  u_int64_t _opaque[9];
} UnotAttrs;

typedef int UnotConnection;
typedef unsigned long UnotID;

UnotConnection unot_connect(const char *sock_path);
void unot_disconnect(UnotConnection conn);

void unot_attr_set_text(UnotAttrs *attrs, const char *value);
void unot_attr_set_indicator(UnotAttrs *attrs, const char *value);
void unot_attr_set_indicator_name(UnotAttrs *attrs, const char *value);
void unot_attr_set_text_font(UnotAttrs *attrs, const char *value);
void unot_attr_set_text_fg(UnotAttrs *attrs, unsigned long value);
void unot_attr_set_indicator_fg(UnotAttrs *attrs, unsigned long value);
void unot_attr_set_timeout(UnotAttrs *attrs, unsigned long value);
void unot_attr_reset(UnotAttrs *attrs);

UnotID unot_notify(UnotConnection conn, UnotAttrs *attrs);
UnotID unot_modify(UnotConnection conn, UnotAttrs *attrs, UnotID id);
UnotID unot_close(UnotConnection conn, UnotID id);

#endif
