#include "list.h"
#include <assert.h>

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

  assert(head && "notification_list_unlink_next: head is NULL");

  NotificationNode *target = NULL;

  if (prev == NULL) {
    target = *head;
    *head = target->next;
    target->next = NULL;
  }

  else {
    target = prev->next;
    if (target) {
      prev->next = target->next;
      target->next = NULL;
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

  NotificationNode *prev = NULL;
  NotificationNode *curr = head;

  while (curr) {
    if (curr->notification.window == window) {
      *previous = prev;
      return curr;
    }
    prev = curr;
    curr = curr->next;
  }

  *previous = NULL;
  return NULL;
}
