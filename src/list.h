#ifndef UNOT_LIST_H
#define UNOT_LIST_H

#include "notification.h"
#include <assert.h>
#include <stdbool.h>

typedef struct _NotificationNode NotificationNode;

struct _NotificationNode {

  Notification notification;
  NotificationNode *next;
};

Notification *notification_list_append(NotificationNode **head,
                                       NotificationNode *node);

NotificationNode *notification_list_unlink_next(NotificationNode **head,
                                                NotificationNode *prev);

void notification_list_remove_next(NotificationNode **head,
                                   NotificationNode *prev);

NotificationNode *
notification_list_find_prev(NotificationNode *head,
                            bool (*predicate)(Notification *));
#endif
