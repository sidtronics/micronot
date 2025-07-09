#ifndef MICRONOT_CLIENT_H
#define MICRONOT_CLIENT_H

#include <stdbool.h>
#include <sys/types.h>

#define UNIndicator (1 << 0)
#define UNIndicatorName (1 << 1)
#define UNTextFG (1 << 2)
#define UNIndicatorFG (1 << 3)
#define UNTextFont (1 << 4)
#define UNTimeout (1 << 5)

typedef struct {
  char *indicator;
  char *indicator_name;
  unsigned long text_fg;
  unsigned long indicator_fg;
  char *text_font;
  unsigned long timeout;
} NotificationAttributes;

typedef unsigned long NotificationID;

int unot_get_connection(const char *sock_path);

bool unot_notify_message(int conn, const char *text, u_int16_t mask,
                         NotificationAttributes *attrs);

bool unot_notify_spinner(int conn, u_int16_t mask,
                         NotificationAttributes *attrs, NotificationID *id);

bool unot_return_spinner(int conn, int retval, NotificationID *id);

void unot_close_connection(int conn);

#endif // !MICRONOT_CLIENT_H
