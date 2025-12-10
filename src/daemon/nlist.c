#include "nlist.h"
#include <assert.h>

bool nlist_empty(NotificationList *list) {

  assert(list && "nlist_empty: list is NULL");
  return list->head == NULL;
}

Notification *nlist_append(NotificationList *list, NotificationNode *node) {

  assert(list && "nlist_append: list is NULL");

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

NotificationNode *nlist_unlink(NotificationList *list, NotificationNode *prev) {

  assert(list && "nlist_unlink: list is NULL");

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

void nlist_remove(NotificationList *list, NotificationNode *prev) {

  NotificationNode *target = nlist_unlink(list, prev);
  free(target);
}

NotificationNode *nlist_find(NotificationList *list, NotificationNode **prev,
                             Window id) {

  assert("nlist_find: list is NULL");

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
