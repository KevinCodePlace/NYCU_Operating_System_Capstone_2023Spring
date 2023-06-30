#include "list.h"

void list_init(list *node) {
    node->next = node;
    node->prev = node;
}

void insert_head(list *head, list *v) {
    v->next = head->next;
    v->prev = head;
    head->next->prev = v;
    head->next = v;
}

void insert_tail(list *head, list *v) {
    v->next = head;
    v->prev = head->prev;
    head->prev->next = v;
    head->prev = v;
}

list *remove_head(list *head) {
    list *ptr;
    ptr = head->next;
    head->next = head->next->next;
    head->next->prev = head;

    return ptr;
}

list *remove_tail(list *head) {
    list *ptr;
    ptr = head->prev;
    head->prev = head->prev->prev;
    head->prev->next = head;

    return ptr;
}

void unlink(list *node) {
    list *next, *prev;
    next = node->next;
    prev = node->prev;

    next->prev = prev;
    prev->next = next;
}