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

Notification *notification_list_append(NotificationList *list,
                                       NotificationNode *node);

NotificationNode *notification_list_unlink_next(NotificationList *list,
                                                NotificationNode *prev);

void notification_list_remove_next(NotificationList *list,
                                   NotificationNode *prev);

NotificationNode *notification_list_find_by_window(NotificationList *list,
                                                   NotificationNode **prev,
                                                   Window window);
#endif
