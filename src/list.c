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

void notification_list_remove_next(NotificationNode *current) {

  if (!current || current->next == NULL)
    return;

  NotificationNode *temp = current->next;
  current->next = temp->next;

  free(temp);
}
