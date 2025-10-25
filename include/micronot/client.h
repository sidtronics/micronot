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

typedef enum { UNOT_TYPE_MESSAGE, UNOT_TYPE_SPINNER } NotificationType;

typedef struct {
  char *indicator;
  unsigned long text_fg;
  unsigned long indicator_fg;
  char *text_font;
  unsigned long timeout;
} NotificationAttributes;

typedef unsigned long NotificationID;

int un_connect(const char *sock_path);

NotificationID un_notify(int conn, char *text, NotificationType type,
                         u_int16_t mask, NotificationAttributes *attrs);

// bool unot_notify(int conn, const char *text, NotificationType type,
//                  u_int16_t mask, NotificationAttributes *attrs,
//                  NotificationID *id);

// bool unot_return(int conn, int retval, NotificationID id);

void un_disconnect(int conn);

#endif
