#include "mini_uart.h"
#include "utils_c.h"
#include "utils_assembly.h"
#include <stddef.h>
#define BUFFER_MAX_SIZE 256u

extern char *_dtb;
void load_img()
{
    char *const kernel_addr = (char *)0x80000;
    uart_send_string("Please sent the kernel image size:");
    char buffer[BUFFER_MAX_SIZE];
    // read_command(buffer);
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
    unsigned int img_size = utils_str2uint_dec(buffer);
    uart_send_string("Start to load the kernel image... \n");

    unsigned char *current = kernel_addr;
    while (img_size--)
    {
        *current = uart_recv_raw();
        current++;
        uart_send('.');
    }
    uart_send_string("loading...\n");
    // branchAddr(kernel_addr);
    ((void (*)(char *))kernel_addr)(_dtb);
}

void bootloader_main(void)
{
    uart_init();
    uart_send_string("In bootloader_main!\n");
    load_img();
}