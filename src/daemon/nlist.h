#ifndef MICRONOT_NLIST_H
#define MICRONOT_NLIST_H

#include "notification.h"

typedef struct _NotificationNode NotificationNode;

typedef struct _NotificationList {
  NotificationNode *head;
  NotificationNode *tail;
} NotificationList;

struct _NotificationNode {
  Notification notification;
  NotificationNode *next;
};

bool nlist_empty(NotificationList *list);

Notification *nlist_append(NotificationList *list, NotificationNode *node);

NotificationNode *nlist_unlink(NotificationList *list, NotificationNode *prev);

void nlist_remove(NotificationList *list, NotificationNode *prev);

NotificationNode *nlist_find(NotificationList *list, NotificationNode **prev,
                             Window id);
#endif
