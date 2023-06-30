#ifndef __LIST_H
#define __LIST_H
#include <stdint.h>
#include <stddef.h>

typedef struct list
{
    struct list *next, *prev;
} list;

#define container_of(ptr, type, member) ({ \
    void *__mptr = (void *)(ptr);          \
    ((type *)(__mptr - offsetof(type, member))); })

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_for_each_entry(entry, head, member)                       \
    for (entry = list_entry((head)->next, __typeof__(*entry), member); \
         &entry->member != (head);                                     \
         entry = list_entry(entry->member.next, __typeof__(*entry), member))

#define LIST_HEAD_INIT(name) \
    {                        \
        &(name), &(name)     \
    }

static inline int list_empty(const list *head)
{
    return head->next == head;
}

void list_init(list *node);
void insert_head(list *head, list *v);
void insert_tail(list *head, list *v);
list *remove_head(list *head);
list *remove_tail(list *head);
void unlink(list *node);
#endif
