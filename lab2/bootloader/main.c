#include "header/uart.h"
#include "header/bootloader.h"

void main()
{
    // set up serial console
    uart_init();
    // say hello
    uart_send_string("Start Bootloading\n");
    load_img();
}
