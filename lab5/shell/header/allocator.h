#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdint.h>

void* simple_malloc(unsigned long size);
void buddy_system_init();

struct node_alloc_node
{
    struct node_alloc_node *next;
    uint32_t size;
};

struct buddy_alloc_node {
    uint32_t idx;
    struct buddy_alloc_node *next;
};

typedef enum
{
    IS_CHILD = -9999,
} buddy_alloc_val;

typedef buddy_alloc_val buddy_alloc_entry;

struct buddy_alloc
{
    unsigned order;
    unsigned long long size;
    unsigned page_size;
    struct node_alloc_node *nhead;
    buddy_alloc_entry *entries;
    struct buddy_alloc_node **freelist;
    void *mem;
};

extern struct buddy_alloc *_global_allocator;
#endif
