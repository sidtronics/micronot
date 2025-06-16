#include "list.h"

Notification *notification_list_append(NotificationNode **head) {

  if (!head)
    return NULL;

  NotificationNode *newNode = malloc(sizeof(NotificationNode));
  if (!newNode)
    return NULL;

  newNode->next = NULL;

  NotificationNode **indirect = head;
  while (*indirect) {
    indirect = &(*indirect)->next;
  }

  *indirect = newNode;
  return &newNode->notification;
}

NotificationNode *notification_list_unlink_next(NotificationNode **head,
                                                NotificationNode *prev) {

  NotificationNode *target = NULL;

  if (prev == NULL) {
    target = *head;
    if (target) {
      *head = target->next;
    }
  } else {
    target = prev->next;
    if (target) {
      prev->next = target->next;
    }
  }

  return target;
}

void notification_list_remove_next(NotificationNode **head,
                                   NotificationNode *prev) {

  NotificationNode *target = notification_list_unlink_next(head, prev);

  if (target) {
    free(target);
  }
}
