#include "list.h"
#include <assert.h>

Notification *notification_list_append(NotificationList *list,
                                       NotificationNode *node) {

  assert(list && "notification_list_append: list is NULL");

  NotificationNode *new_node = (node ? node : malloc(sizeof(NotificationNode)));
  new_node->next = NULL;

  if (list->tail) {
    list->tail->next = new_node;
  }

  else {
    list->head = new_node;
  }

  list->tail = new_node;

  return &new_node->notification;
}

NotificationNode *notification_list_unlink_next(NotificationList *list,
                                                NotificationNode *prev) {

  assert(list && "notification_list_unlink_next: list is NULL");

  NotificationNode *target = NULL;

  if (prev == NULL) {
    target = list->head;
    if (target) {
      list->head = target->next;
      target->next = NULL;
    }
  }

  else {
    target = prev->next;
    if (target) {
      prev->next = target->next;
      target->next = NULL;
    }
  }

  if (target == list->tail) {
    list->tail = prev;
  }

  return target;
}

void notification_list_remove_next(NotificationList *list,
                                   NotificationNode *prev) {

  NotificationNode *target = notification_list_unlink_next(list, prev);

  if (target) {
    free(target);
  }
}

NotificationNode *notification_list_find_by_window(NotificationList *list,
                                                   NotificationNode **prev,
                                                   Window window) {

  NotificationNode *previous = NULL;
  NotificationNode *current = list->head;

  while (current) {
    if (current->notification.window == window) {
      *prev = previous;
      return current;
    }
    previous = current;
    current = current->next;
  }

  *prev = NULL;
  return NULL;
}
