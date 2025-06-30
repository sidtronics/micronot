#include "list.h"
#include "utils.h"

bool notification_list_is_empty(NotificationList *list) {

  pthread_mutex_lock(&list->lock);
  bool res = (list->head == NULL);
  pthread_mutex_unlock(&list->lock);

  return res;
}

Notification *notification_list_append(NotificationList *list,
                                       NotificationNode *node) {

  ASSERT(list && "notification_list_append: list is NULL");

  NotificationNode *new_node = (node ? node : malloc(sizeof(NotificationNode)));
  new_node->next = NULL;

  pthread_mutex_lock(&list->lock);
  if (list->tail) {
    list->tail->next = new_node;
  }

  else {
    list->head = new_node;
  }

  list->tail = new_node;
  pthread_mutex_unlock(&list->lock);

  return &new_node->notification;
}

NotificationNode *notification_list_unlink(NotificationList *list,
                                           NotificationNode *prev) {

  ASSERT(list && "notification_list_unlink_next: list is NULL");

  NotificationNode *target = NULL;

  pthread_mutex_lock(&list->lock);
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
  pthread_mutex_unlock(&list->lock);

  return target;
}

void notification_list_remove(NotificationList *list, NotificationNode *prev) {

  NotificationNode *target = notification_list_unlink(list, prev);
  free(target);
}

void notification_list_foreach(NotificationList *list, NotificationNode *prev,
                               NotificationVisitor visit, void *data) {

  ASSERT(list && "notification_list_foreach: list is NULL");
  ASSERT(visit && "notification_list_foreach: callback is NULL");

  pthread_mutex_lock(&list->lock);
  NotificationNode *previous = prev;
  NotificationNode *current = (prev ? prev->next : list->head);
  while (current) {

    previous ? visit(&previous->notification, &current->notification, data)
             : visit(NULL, &current->notification, data);

    previous = current;
    current = current->next;
  }
  pthread_mutex_unlock(&list->lock);
}

NotificationNode *notification_list_find(NotificationList *list,
                                         NotificationNode **prev,
                                         NotificationPredicate match,
                                         void *data) {

  pthread_mutex_lock(&list->lock);
  NotificationNode *previous = NULL;
  NotificationNode *current = list->head;

  while (current) {
    if (match(&current->notification, data)) {
      *prev = previous;

      pthread_mutex_unlock(&list->lock);
      return current;
    }
    previous = current;
    current = current->next;
  }

  *prev = NULL;

  pthread_mutex_unlock(&list->lock);
  return NULL;
}
