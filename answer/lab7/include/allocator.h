#pragma once
#include "uint.h"
void* simple_malloc(unsigned int size);
void* malloc(size_t size);
void* getbestfit(int ind);
void show_status();
void init_allocator();
void free(void* address);
void pool_status();
void clear_pool();
void memory_reserve(uint64 start,uint64 end);

struct FrameArray{
    int val, index;
    struct FrameArray* next;
    int allocatable;
};

struct mem_frag{
    void* start,*end;
    struct mem_frag *next;
    uint32 size, num, leave;
    byte *status;
};


struct mem_reserved_pool{
    void* start, *end;
    struct mem_reserved_pool *next;
};