#include "utils.h"
#include <stddef.h>

/* 

    string part

*/
int utils_str_compare(char *a, char *b)
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
void utils_uint2str_hex(unsigned int num, char *str)
{
    // num=7114 digit=4
    unsigned int temp = num;
    int digit = -1;
    *str = '0';
    *str++;
    *str = 'x';
    *str++;
    if (num == 0)
    {
        *str = '0';
        str++;
    }
    else
    {
        while (temp > 0)
        {
            temp /= 16;
            digit++;
        }
        for (int i = digit; i >= 0; i--)
        {
            int t = 1;
            for (int j = 0; j < i; j++)
            {
                t *= 16;
            }
            if (num / t >= 10)
            {
                *str = '0' + num / t + 39;
            }
            else
            {
                *str = '0' + num / t;
            }
            num = num % t;
            str++;
        }
    }
    *str = '\0';
}

/* 

    reboot part

*/

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}