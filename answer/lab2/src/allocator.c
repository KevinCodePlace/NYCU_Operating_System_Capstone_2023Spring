#include "allocator.h"
#include "utils_c.h"
#include "mini_uart.h"

#define MEM_SIZE 0x10000000 // 0.25G
#define MEM_START 0x10000000

unsigned long *malloc_cur = (unsigned long *)MEM_START;

void *malloc(size_t size)
{
    align(&size,4);//allocated the memory size is mutiple of 4 byte;
    unsigned long *malloc_ret = malloc_cur;
    malloc_cur+=(unsigned int)size;
    return malloc_ret;
}