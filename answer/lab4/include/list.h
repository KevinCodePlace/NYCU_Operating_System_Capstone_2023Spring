#ifndef _LIST_H_
#define _LIST_H_
#include <stdint.h>
#include <stddef.h>

typedef struct list {
    struct list *next, *prev;
}list;

void list_init(list *node);
void insert_head(list *head, list *v);
void insert_tail(list *head, list *v);
list *remove_head(list *head);
list *remove_tail(list *head);
void remove(list *node);
#endif
