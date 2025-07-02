#ifndef UNOT_LIST_H
#define UNOT_LIST_H

#include "notification.h"
#include <pthread.h>
#include <stdbool.h>

typedef struct _NotificationNode NotificationNode;

typedef struct _NotificationList {
  NotificationNode *head;
  NotificationNode *tail;
} NotificationList;

struct _NotificationNode {

  Notification notification;
  NotificationNode *next;
};

bool notification_list_is_empty(NotificationList *list);

Notification *notification_list_append(NotificationList *list,
                                       NotificationNode *node);

NotificationNode *notification_list_unlink(NotificationList *list,
                                           NotificationNode *prev);

void notification_list_remove(NotificationList *list, NotificationNode *prev);


NotificationNode *notification_list_find(NotificationList *list,
                                         NotificationNode **prev, Window id);
#endif
