
#include "header/bootloader.h"
#include "header/uart.h"

void load_img(){
    unsigned int size = 0;
    unsigned char *size_buffer = (unsigned char *) &size;
    for(int i=0; i<4; i++) 
	    size_buffer[i] = uart_get_char();
    uart_send_string("size-check correct\n");

    char *kernel = (char *) 0x80000;
    while(size--) *kernel++ = uart_get_char();

    uart_send_string("kernel-loaded\n");
    
    asm volatile(
       "mov x30, 0x80000;"
       "ret;"
    );

}
