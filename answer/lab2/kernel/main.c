#include "mini_uart.h"
#include "dtb.h"
#include "shell.h"

extern void *_dtb_ptr;

void kernel_main(void)
{
    // uart_init();
    uart_send_string("Hello, world!\n");
    fdt_traverse(get_initramfs_addr,_dtb_ptr);
    shell();
}