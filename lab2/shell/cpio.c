#include "header/cpio.h"
#include "header/uart.h"
#include "header/utils.h"

unsigned long atoi(const char *s, int char_size) {
	unsigned long num = 0;
	for(int i=0;i<char_size;i++){
		num = (num + (*s-'0')) << 4;
		s++;
	}
	num = num / 16;
	return num;
}

void align(void *size, unsigned int s) {
	unsigned long* x = (unsigned long*) size;
	*x = ((*x) + (s-1)) & (~(s-1));
}

void cpio_ls(){
	char* addr = (char*) QEMU_CPIO_ADDR;

	for(int i=0;i<535;i++) {
		uart_send_char(*addr);
		//if(*addr == '\0') {
		//		uart_send_char('\n');
		//}
		addr++;
	}
    
	uart_send_char('\n');
	addr = QEMU_CPIO_ADDR;
	while(string_compare((char *)(addr+sizeof(struct cpio_header)),"TRAILER!!!") == 0){
		
		struct cpio_header* header = (struct cpio_header*) addr;
		unsigned long filename_size = atoi(header->c_namesize,(int)sizeof(header->c_namesize));
		uart_send_string("\nFilenamesize:");
		uart_binary_to_hex(filename_size);
		uart_send_char('\n');
		unsigned long headerPathname_size = sizeof(struct cpio_header) + filename_size;
		unsigned long file_size = atoi(header->c_filesize,(int)sizeof(header->c_filesize));
	    uart_send_string("file_size:");
		uart_binary_to_hex(file_size);
		uart_send_char('\n');
		align(&headerPathname_size,4);
		align(&file_size,4);
		
		uart_send_string("Filename:");
		uart_send_string(addr+sizeof(struct cpio_header));
		uart_send_string("\n");
		
		addr += (headerPathname_size + file_size);
	}
}
