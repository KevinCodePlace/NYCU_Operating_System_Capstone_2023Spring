#include "allocator.h"
#include "buffer.h"


int write_buffer(struct buffer *b, unsigned char c){
    if(b->start-1  == b->end || (b->start == buffer_size-1 && b->end == 0)) return 0;
    b->buf[b->end++] = c;
    if(b->end == buffer_size) b->end=0;
    return 1;
}

int read_buffer(struct buffer *b, unsigned char* output){
    if(b->start == b->end) return 0;
    *output = b->buf[b->start];
    b->buf[b->start++] = 0;
    if(b->start == buffer_size) b->start=0;
    return 1;
}

