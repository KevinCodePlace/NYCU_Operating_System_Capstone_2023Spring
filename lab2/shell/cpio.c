#include "header/cpio.h"
#include "header/uart.h"

void cpio_ls(){
	char *ptr = (char *)QEMU_CPIO_ADDR;
	for (int i=0;i<1000;i++) {
	   uart_send_char(*ptr);
	   ptr++;
	   
	}

}
