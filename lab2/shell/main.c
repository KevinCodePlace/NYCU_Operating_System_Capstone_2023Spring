#include "header/uart.h"
#include "header/shell.h"
#include "header/dtb.h"
#include "header/utils.h"
extern void *_dtb_ptr;

void main()
{
    // set up serial console
    uart_init();
	
    // say hello
	//uintptr_t dtb_ptr = (uintptr_t) _dtb_ptr;
	//utils_align_up(dtb_ptr,8);
	//uart_send_string("\ndtb loading at: ");
	//uart_binary_to_hex(dtb_ptr);
	//uart_send_char('\n');
	//fdt_traverse(_dtb_ptr);
    uart_send_string("Type in `help` to get instruction menu!\n");
    //echo everything back
	shell();
}
