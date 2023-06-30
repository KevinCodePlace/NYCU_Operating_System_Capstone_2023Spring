#ifndef _buffer_
#define _buffer_
#define buffer_size 1024
struct buffer{
    char buf[buffer_size];
    int start,end;
};

int write_buffer(struct buffer*,unsigned char c);
int read_buffer(struct buffer*,unsigned char *c);

#endif