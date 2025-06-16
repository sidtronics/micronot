#ifndef UNOT_LIST_H
#define UNOT_LIST_H

#include "notification.h"

typedef struct _NotificationNode NotificationNode;

struct _NotificationNode {

  Notification notification;
  NotificationNode *next;
};

Notification *notification_list_append(NotificationNode **head);

NotificationNode *notification_list_unlist_next(NotificationNode *current);

void notification_list_remove_next(NotificationNode *current);

#endif
