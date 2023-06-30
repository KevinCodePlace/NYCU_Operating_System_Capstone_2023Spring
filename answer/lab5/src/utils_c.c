#include "utils_c.h"
#include "mini_uart.h"
#include <stddef.h>

/*
    string part
*/

int utils_str_compare(const char *a, const char *b)
{
    char aa, bb;
    do
    {
        aa = (char)*a++;
        bb = (char)*b++;
        if (aa == '\0' || bb == '\0')
        {
            return aa - bb;
        }
    } while (aa == bb);
    return aa - bb;
}

int utils_strncmp(const char *a, const char *b, size_t n)
{
    size_t i = 0;
    while (i < n - 1 && a[i] == b[i] && a[i] != '\0' && b[i] != '\0')
        i++;
    return a[i] - b[i];
}
void utils_newline2end(char *str)
{
    while (*str != '\0')
    {
        if (*str == '\n')
        {
            *str = '\0';
            return;
        }
        ++str;
    }
}

void utils_int2str_dec(int num, char *str)
{
    // num=7114 digit=4
    int digit = -1, temp = num;
    while (temp > 0)
    {
        temp /= 10;
        digit++;
    }
    for (int i = digit; i >= 0; i--)
    {
        int t = 1;
        for (int j = 0; j < i; j++)
        {
            t *= 10;
        }
        *str = '0' + num / t;
        num = num % t;
        str++;
    }
    *str = '\0';
}

void utils_uint2str_dec(unsigned int num, char *str)
{
    // num=7114 digit=4
    unsigned int temp = num;
    int digit = -1;
    while (temp > 0)
    {
        temp /= 10;
        digit++;
    }
    for (int i = digit; i >= 0; i--)
    {
        int t = 1;
        for (int j = 0; j < i; j++)
        {
            t *= 10;
        }
        *str = '0' + num / t;
        num = num % t;
        str++;
    }
    *str = '\0';
}

unsigned int utils_str2uint_dec(const char *str)
{
    unsigned int value = 0u;

    while (*str)
    {
        value = value * 10u + (*str - '0');
        ++str;
    }
    return value;
}

size_t utils_strlen(const char *s)
{
    size_t i = 0;
    while (s[i])
        i++;
    return i + 1;
}

/*
    reboot part
*/

void set(long addr, unsigned int value)
{
    volatile unsigned int *point = (unsigned int *)addr;
    *point = value;
}

void reset(int tick)
{                                     // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20); // full reset
    set(PM_WDOG, PM_PASSWORD | tick); // number of watchdog tick
}

void cancel_reset()
{
    set(PM_RSTC, PM_PASSWORD | 0); // full reset
    set(PM_WDOG, PM_PASSWORD | 0); // number of watchdog tick
}

/*
    others
*/

void align(void *size, size_t s)
{
    unsigned int *x = (unsigned int *)size;
    if ((*x) & (s - 1))
    {
        (*x) += s - ((*x) & (s - 1));
    }
}

uint32_t align_up(uint32_t size, int alignment)
{
    return (size + alignment - 1) & -alignment;
}

void delay(unsigned int clock)
{
    while (clock--)
    {
        asm volatile("nop");
    }
}

void memcpy(void *dst, const void *src, size_t n)
{
    char *_dst = dst;
    const char *_src = src;

    while (n--)
    {
        *_dst++ = *_src++;
    }
}

void *memset(void *s, int c, size_t n) {
  char *p = s;
  for (size_t i = 0; i < n; i++) {
    p[i] = c;
  }
  return s;
}