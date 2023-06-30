#include "mini_uart.h"
#include "dtb.h"
#include "exception_c.h"
#include "utils_s.h"
#include "shell.h"
#include "mm.h"
#include "timer.h"

extern void *_dtb_ptr;

void kernel_main(void)
{
    // uart_init();
    timeout_event_init();
    mm_init();
    uart_send_string("Hello, world!\n");
    int el = get_el();
    uart_printf("kernel Exception level: %d\n",el);
    enable_interrupt();
    shell();
}