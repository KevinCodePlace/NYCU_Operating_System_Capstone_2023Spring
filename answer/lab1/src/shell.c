#include "shell.h"
#include "mini_uart.h"
#include "utils.h"
#include "peripheral/mailbox.h"
#include <stddef.h>
#define BUFFER_MAX_SIZE 256u
#define COMMNAD_LENGTH_MAX 20u

static const char *command_list[] = {"help", "hello", "reboot", "info"};
static const char *command_explain[] = {"print this help menu\r\n", "print Hello World!\r\n", "reboot the device\r\n", "the mailbox hardware info\r\n"};

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
}

void help()
{
    uart_send_string("help     :");
    uart_send_string("print this help menu\r\n");
    uart_send_string("hello    :");
    uart_send_string("print Hello World!\r\n");
    uart_send_string("reboot   :");
    uart_send_string("reboot the device\r\n");
    uart_send_string("info     :");
    uart_send_string("the mailbox hardware info\r\n");

    // for (size_t i = 0; i < sizeof(command_list) / sizeof(const char *); i++)
    // {
    //     uart_hex(i);
    //     uart_send_string(command_list[i]);
    //     // int command_len = 0;
    //     // while (command_list[i][command_len] != '\0')
    //     // {
    //     //     command_len++;
    //     // }
    //     // for (int k = COMMNAD_LENGTH_MAX - command_len; k >= 0; k--)
    //     // {
    //     //     uart_send(' ');
    //     // }

    //     uart_send_string(":");
    //     uart_send_string(command_explain[i]);
    // }
}

void hello()
{
    uart_send_string("Hello World!\r\n");
}

void info()
{
    get_board_revision();
    get_arm_memory();
}

void parse_command(char *buffer)
{
    utils_newline2end(buffer);
    uart_send('\r');

    if (buffer[0] == '\0')
    { // enter empty
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
    else if (utils_str_compare(buffer, "reboot") == 0)
    {
        uart_send_string("rebooting...\r\n");
        reset(1000);
    }
    else if (utils_str_compare(buffer, "info") == 0)
    {
        info();
    }
    else
    {
        uart_send_string("commnad '");
        uart_send_string(buffer);
        uart_send_string("' not found\r\n");
    }
}

void shell()
{
    while (1)
    {
        char buffer[BUFFER_MAX_SIZE];
        uart_send_string("$ ");
        read_command(buffer);
        parse_command(buffer);
    }
}