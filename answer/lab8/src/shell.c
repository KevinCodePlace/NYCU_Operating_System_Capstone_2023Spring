#include "shell.h"
#include "mini_uart.h"
#include "utils_c.h"
#include "utils_s.h"
#include "peripheral/mailbox.h"
#include "_cpio.h"
#include "exception_c.h"
#include "timer.h"
#include "dtb.h"
#include "mm.h"
#include "exec.h"
#include "thread.h"
#include <stddef.h>
#include "vfs.h"
#define BUFFER_MAX_SIZE 256u
#define COMMNAD_LENGTH_MAX 20u

extern void *_dtb_ptr;

int split(const char *buf, char *outbuf[], int n)
{
    const char *ps, *pe;
    int idx = 0;
    ps = pe = buf;

    while (idx < n)
    {
        while (*pe && *pe != ' ')
        {
            pe++;
        }

        int size = pe - ps;
        if (size)
        {
            outbuf[idx] = kmalloc(size + 1);
            memcpy(outbuf[idx], ps, size);
            outbuf[idx][size] = '\0';
            idx++;
        }

        if (*pe)
        {
            while (*pe == ' ')
                pe++;
            ps = pe;
        }
        else
        {
            break;
        }
    }

    return idx;
}

void read_command(char *buffer)
{
    size_t index = 0;
    while (1)
    {
        buffer[index] = uart_recv();
        uart_send(buffer[index]);
        if (buffer[index] == '\n')
        {
            break;
        }
        index++;
    }
    buffer[index + 1] = '\0';
    utils_newline2end(buffer);
    uart_send('\r');
}

void parse_arg(char *buffer, int *argi)
{
    int i = 0;
    int argi_index = 0;
    argi[argi_index] = i;
    while (buffer[i] != '\0')
    {
        if (buffer[i] == ' ')
        {
            buffer[i] = '\0';
            argi[++argi_index] = ++i;
            continue;
        }
        i++;
    }
}

void help()
{
    uart_send_string("help    :");
    uart_send_string("print this help menu\n");
    uart_send_string("hello   :");
    uart_send_string("print Hello World!\n");
    uart_send_string("reboot  : ");
    uart_send_string("reboot the device\n");
    uart_send_string("info    : ");
    uart_send_string("the mailbox hardware info\n");
    uart_send_string("ls      : ");
    uart_send_string("list the all file\n");
    uart_send_string("cat     : ");
    uart_send_string("print the file content\n");
    uart_send_string("dtb     : ");
    uart_send_string("print the device name tree \n");
    uart_send_string("async   : ");
    uart_send_string("test uart async send and recv\n");
    uart_send_string("set     : ");
    uart_send_string("set the timeout (set message second)\n");
    uart_send_string("buddy   : ");
    uart_send_string("test the page allocate and free\n");
    uart_send_string("dynamic : ");
    uart_send_string("test the dynamic memory simple_allocator\n");
}

void hello()
{
    uart_send_string("Hello World!\n");
}

void info()
{
    get_board_revision();
    // get_arm_memory();
}

void parse_command(char *buffer)
{
    // int argi[BUFFER_MAX_SIZE];
    if (utils_str_compare(buffer, "") == 0)
    {
        return;
    }
    else if (utils_str_compare(buffer, "help") == 0)
    {
        help();
    }
    else if (utils_str_compare(buffer, "hello") == 0)
    {
        hello();
    }
    else if (utils_str_compare(buffer, "ls") == 0)
    {
        cpio_ls();
    }
    else if (utils_str_compare(buffer, "cat") == 0)
    {
        uart_send_string("Filename: ");
        char buffer[BUFFER_MAX_SIZE];
        read_command(buffer);
        cpio_cat(buffer);
    }
    else if (utils_str_compare(buffer, "exe") == 0)
    {
        exe_new_prog("/initramfs/user2.img");
    }
        else if (utils_str_compare(buffer, "wtf") == 0)
    {
        uart_printf("Current task count :%d\n",get_the_cur_count());
    }
    else if (utils_str_compare(buffer, "thread") == 0)
    {
        test_thread();
        uart_async_send_string("exit the test_thread\n");
    }
    else if (utils_str_compare(buffer, "reboot") == 0)
    {
        uart_send_string("rebooting...\n");
        reset(1000);
    }
    else if (utils_str_compare(buffer, "info") == 0)
    {
        info();
    }
    else if (utils_str_compare(buffer, "dtb") == 0)
    {
        fdt_traverse(print_dtb);
    }
        else if (utils_str_compare(buffer, "fs") == 0)
    {
        vfs_test();
    }
    else if (utils_str_compare(buffer, "async") == 0)
    {
        test_uart_async();
    }
    else if (utils_strncmp(buffer, "set", 3) == 0)
    {
        char *args[3];
        split(buffer, args, 3);
        unsigned long time = (unsigned long)utils_str2uint_dec(args[2]);
        set_timeout(args[1], S(time));
    }
    else if (utils_str_compare(buffer, "buddy") == 0)
    {
        test_buddy();
    }
    else if (utils_str_compare(buffer, "dynamic") == 0)
    {
        test_dynamic_alloc();
    }
    else
    {
        uart_send_string("commnad '");
        uart_send_string(buffer);
        uart_send_string("' not found\n");
    }
}

void clear_buffer(char *buf)
{
    for (int i = 0; i < BUFFER_MAX_SIZE; i++)
    {
        buf[i] = '\0';
    }
}

void shell()
{
    while (1)
    {
        char buffer[BUFFER_MAX_SIZE];
        clear_buffer(buffer);
        uart_send_string("$ ");
        read_command(buffer);
        parse_command(buffer);
    }
}