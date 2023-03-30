#include "header/cpio.h"
#include "header/uart.h"
#include "header/utils.h"


unsigned long atoi(const char *s, int char_size) {
    unsigned long num = 0;
    for (int i = 0; i < char_size; i++) {
        num = num * 16;
        if (*s >= '0' && *s <= '9') {
            num += (*s - '0');
        } else if (*s >= 'A' && *s <= 'F') {
            num += (*s - 'A' + 10);
        } else if (*s >= 'a' && *s <= 'f') {
            num += (*s - 'a' + 10);
        }
        s++;
    }
    return num;
}

void align(void *size, unsigned int s) {
	unsigned long* x = (unsigned long*) size;
	*x = ((*x) + (s-1)) & (~(s-1));
}

void cpio_ls(){
	char* addr = (char*) QEMU_CPIO_ADDR;

	while(string_compare((char *)(addr+sizeof(struct cpio_header)),"TRAILER!!!") == 0){
		

		struct cpio_header* header = (struct cpio_header*) addr;
		unsigned long filename_size = atoi(header->c_namesize,(int)sizeof(header->c_namesize));
		unsigned long headerPathname_size = sizeof(struct cpio_header) + filename_size;
		unsigned long file_size = atoi(header->c_filesize,(int)sizeof(header->c_filesize));
	    
		align(&headerPathname_size,4);
		align(&file_size,4);
		
		uart_send_string(addr+sizeof(struct cpio_header));
		uart_send_string("\n");
		
		addr += (headerPathname_size + file_size);
	}
}
