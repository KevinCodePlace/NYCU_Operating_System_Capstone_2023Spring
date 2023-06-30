#ifndef _UTILS_C_H
#define _UTILS_C_H

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024
#include <stddef.h>
#include <stdint.h>

/* string */
int utils_str_compare(const char *a, const char *b);
int utils_strncmp(const char *a, const char *b, size_t n);
void utils_newline2end(char *str);
char utils_int2char(int a);
void utils_int2str_dec(int a, char *str);
void utils_uint2str_dec(unsigned int num, char *str);
unsigned int utils_str2uint_dec(const char *str);
void align(void *size, size_t s); // aligned to 4 byte
size_t utils_strlen(const char *s);
uint32_t align_up(uint32_t size, int alignment);

/* reboot */
void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();

/* others */
void delay(unsigned int clock);
void memcpy(void *dst, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

#endif
