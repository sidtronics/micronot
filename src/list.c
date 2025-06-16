#include "list.h"

Notification *notification_list_append(NotificationNode **head) {

  if (head == NULL)
    return NULL;

  NotificationNode *newNode =
      (NotificationNode *)malloc(sizeof(NotificationNode));

  newNode->next = NULL;

  if (*head == NULL) {
    *head = newNode;
  }

  else {
    NotificationNode *current = *head;

    while (current->next)
      current = current->next;

    current->next = newNode;
  }

  return &newNode->notification;
}

NotificationNode *notification_list_unlist_next(NotificationNode *current) {

  if (!current || current->next == NULL)
    return NULL;

  NotificationNode *temp = current->next;
  current->next = temp->next;

  return temp;
}

void notification_list_remove_next(NotificationNode *current) {

  NotificationNode *target = notification_list_unlist_next(current);
  if (target == NULL)
    return;

  free(target);
}
