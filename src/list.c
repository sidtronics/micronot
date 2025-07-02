#include "list.h"
#include "utils.h"

bool notification_list_is_empty(NotificationList *list) {

  return list->head == NULL;
}

Notification *notification_list_append(NotificationList *list,
                                       NotificationNode *node) {

  ASSERT(list && "notification_list_append: list is NULL");

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

NotificationNode *notification_list_unlink(NotificationList *list,
                                           NotificationNode *prev) {

  ASSERT(list && "notification_list_unlink_next: list is NULL");

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

void notification_list_remove(NotificationList *list, NotificationNode *prev) {

  NotificationNode *target = notification_list_unlink(list, prev);
  free(target);
}

NotificationNode *notification_list_find(NotificationList *list,
                                         NotificationNode **prev, Window id) {

  NotificationNode *previous = NULL;
  NotificationNode *current = list->head;

  while (current) {
    if (current->notification.window == id) {
      *prev = previous;
      return current;
    }
    previous = current;
    current = current->next;
  }

  *prev = NULL;
  return NULL;
}
