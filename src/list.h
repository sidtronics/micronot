#ifndef UNOT_LIST_H
#define UNOT_LIST_H

#include "notification.h"
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

typedef void (*NotificationVisitor)(Notification *prev, Notification *curr,
                                    void *data);

typedef bool (*NotificationPredicate)(Notification *node, void *data);

Notification *notification_list_append(NotificationList *list,
                                       NotificationNode *node);

NotificationNode *notification_list_unlink_next(NotificationList *list,
                                                NotificationNode *prev);

void notification_list_remove_next(NotificationList *list,
                                   NotificationNode *prev);

void notification_list_foreach(NotificationList *list, NotificationNode *prev,
                               NotificationVisitor visit, void *data);

NotificationNode *notification_list_find(NotificationList *list,
                                         NotificationNode **prev,
                                         NotificationPredicate match,
                                         void *data);

#endif
