#ifndef _UTILS_H
#define _UTILS_H

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

/* string */

int utils_str_compare(char* a, char* b);
void utils_newline2end(char *str);
char utils_int2char(int a);
void utils_int2str_dec(int a,char* str);
void utils_uint2str_dec(unsigned int num,char* str);
void utils_uint2str_hex(unsigned int num,char* str);

/* reboot */
void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();

#endif

