#include "list.h"

Notification *notification_list_append(NotificationNode **head,
                                       NotificationNode *node) {

  if (head == NULL)
    return NULL;

  NotificationNode *new_node = (node ? node : malloc(sizeof(NotificationNode)));

  new_node->next = NULL;

  NotificationNode **indirect = head;
  while (*indirect) {
    indirect = &(*indirect)->next;
  }

  *indirect = new_node;
  return &new_node->notification;
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

NotificationNode *notification_list_find_by_window(NotificationNode *head,
                                                   NotificationNode **previous,
                                                   Window window) {

  NotificationNode *prev, *curr;

  for (curr = head; curr; prev = curr, curr = curr->next) {
    if (curr->notification.window == window) {
      if (previous)
        *previous = prev;
      return curr;
    }
  }

  if (previous)
    *previous = NULL;

  return NULL;
}
