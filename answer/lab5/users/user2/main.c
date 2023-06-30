#include <stddef.h>
#include <stdarg.h>

#define SYS_GETPID 0
#define SYS_UART_RECV 1
#define SYS_UART_WRITE 2
#define SYS_EXEC 3
#define SYS_FORK 4
#define SYS_EXIT 5
#define SYS_MBOX_CALL 6
#define SYS_KILL_PID 7

int start(void) __attribute__((section(".start")));

unsigned long syscall(unsigned long syscall_num,
                      void *x0,
                      void *x1,
                      void *x2,
                      void *x3,
                      void *x4,
                      void *x5)
{
    unsigned long result;

    asm volatile(
        "ldr x8, %0\n"
        "ldr x0, %1\n"
        "ldr x1, %2\n"
        "ldr x2, %3\n"
        "ldr x3, %4\n"
        "ldr x4, %5\n"
        "ldr x5, %6\n"
        "svc 0\n" ::"m"(syscall_num),
        "m"(x0), "m"(x1),
        "m"(x2), "m"(x3), "m"(x4), "m"(x5));

    asm volatile(
        "str x0, %0\n"
        : "=m"(result));

    return result;
}
/* system call */
int getpid()
{
    return (int)syscall(SYS_GETPID, 0, 0, 0, 0, 0, 0);
}

void uart_recv(const char buf[], size_t size)
{
    syscall(SYS_UART_RECV, (void *)buf, (void *)size, 0, 0, 0, 0);
}

void uart_write(const char buf[], size_t size)
{
    syscall(SYS_UART_WRITE, (void *)buf, (void *)size, 0, 0, 0, 0);
}

void exit(void)
{
    syscall(SYS_EXIT, 0, 0, 0, 0, 0, 0);
}
void kill_pid(unsigned long pid)
{
    syscall(SYS_KILL_PID, (void *)pid, 0, 0, 0, 0, 0);
}
void mailbox_call(unsigned char ch, unsigned int *mbox)
{
    syscall(SYS_MBOX_CALL, (void *)(unsigned long)ch, mbox, 0, 0, 0, 0);
}
void exec(const char *name, char *const argv[])
{
    syscall(SYS_EXEC, (void *)name, (void *)argv, 0, 0, 0, 0);
}
int fork(void)
{
    return (int)syscall(SYS_FORK, 0, 0, 0, 0, 0, 0);
}
/* normal function */

static void uart_send(char c)
{
    uart_write(&c, 1);
}

void uart_send_string(const char *str)
{
    while (*str)
    {
        uart_send(*str++);
    }
}

unsigned int vsprintf(char *dst, char *fmt, __builtin_va_list args)
{
    long int arg;
    int len, sign, i;
    char *p, *orig = dst, tmpstr[19];

    // failsafes
    if (dst == (void *)0 || fmt == (void *)0)
    {
        return 0;
    }

    // main loop
    arg = 0;
    while (*fmt)
    {
        // argument access
        if (*fmt == '%')
        {
            fmt++;
            // literal %
            if (*fmt == '%')
            {
                goto put;
            }
            len = 0;
            // size modifier
            while (*fmt >= '0' && *fmt <= '9')
            {
                len *= 10;
                len += *fmt - '0';
                fmt++;
            }
            // skip long modifier
            if (*fmt == 'l')
            {
                fmt++;
            }
            // character
            if (*fmt == 'c')
            {
                arg = __builtin_va_arg(args, int);
                *dst++ = (char)arg;
                fmt++;
                continue;
            }
            else
                // decimal number
                if (*fmt == 'd')
                {
                    arg = __builtin_va_arg(args, int);
                    // check input
                    sign = 0;
                    if ((int)arg < 0)
                    {
                        arg *= -1;
                        sign++;
                    }
                    if (arg > 99999999999999999L)
                    {
                        arg = 99999999999999999L;
                    }
                    // convert to string
                    i = 18;
                    tmpstr[i] = 0;
                    do
                    {
                        tmpstr[--i] = '0' + (arg % 10);
                        arg /= 10;
                    } while (arg != 0 && i > 0);
                    if (sign)
                    {
                        tmpstr[--i] = '-';
                    }
                    // padding, only space
                    if (len > 0 && len < 18)
                    {
                        while (i > 18 - len)
                        {
                            tmpstr[--i] = ' ';
                        }
                    }
                    p = &tmpstr[i];
                    goto copystring;
                }
                else
                    // hex number
                    if (*fmt == 'x')
                    {
                        arg = __builtin_va_arg(args, long int);
                        // convert to string
                        i = 16;
                        tmpstr[i] = 0;
                        do
                        {
                            char n = arg & 0xf;
                            // 0-9 => '0'-'9', 10-15 => 'A'-'F'
                            tmpstr[--i] = n + (n > 9 ? 0x37 : 0x30);
                            arg >>= 4;
                        } while (arg != 0 && i > 0);
                        // padding, only leading zeros
                        if (len > 0 && len <= 16)
                        {
                            while (i > 16 - len)
                            {
                                tmpstr[--i] = '0';
                            }
                        }
                        p = &tmpstr[i];
                        goto copystring;
                    }
                    else
                        // string
                        if (*fmt == 's')
                        {
                            p = __builtin_va_arg(args, char *);
                        copystring:
                            if (p == (void *)0)
                            {
                                p = "(null)";
                            }
                            while (*p)
                            {
                                *dst++ = *p++;
                            }
                        }
        }
        else
        {
        put:
            *dst++ = *fmt;
        }
        fmt++;
    }
    *dst = 0;
    // number of bytes written
    return dst - orig;
}

/**
 * Variable length arguments
 */
unsigned int sprintf(char *dst, char *fmt, ...)
{
    //__builtin_va_start(args, fmt): "..." is pointed by args
    //__builtin_va_arg(args,int): ret=(int)*args;args++;return ret;
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    return vsprintf(dst, fmt, args);
}
unsigned int uart_printf(char *fmt, ...)
{
    char dst[100];
    // __builtin_va_start(args, fmt): "..." is pointed by args
    // __builtin_va_arg(args,int): ret=(int)*args;args++;return ret;
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    unsigned int ret = vsprintf(dst, fmt, args);
    uart_send_string(dst);
    return ret;
}

#define MAILBOX_CH_PROP 8
#define REQUEST_CODE 0x00000000
#define GET_BOARD_REVISION 0x00010002
#define TAG_REQUEST_CODE 0x00000000
#define END_TAG 0x00000000

unsigned int __attribute__((aligned(16))) mailbox[8];

unsigned int get_board_revision()
{
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION; // tag identifier
    mailbox[3] = 4;                  // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;

    mailbox_call(MAILBOX_CH_PROP, mailbox); // message passing procedure call, you should implement it following the 6 steps provided above.
    return mailbox[5];
}

int start(void)
{
    // char buf1[0x10] = {0};
    int pid = getpid();
    uart_printf("[User2] pid:%d\n", pid);

    unsigned int revision = get_board_revision();
    uart_printf("[User2] Revision: %x\n", revision);

    pid = fork();

    if (pid == 0)
    {
        uart_printf("[User2] child: exec user1.img\r\n");
        exec("user1.img", NULL);
    }
    else
    {
        uart_printf("[User2] parent: child pid: %d\n", pid);
    }
    uart_printf("[User2 ] exit\n");
    exit();
    return 0;
}
