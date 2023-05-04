#include "header/cpio.h"
#include "header/uart.h"
#include "header/utils.h"
#include "header/allocator.h"

int file_num = 0;
struct file *f = NULL;

void allocate_file_array() {
    if (f == NULL) {
        f = (struct file *)simple_malloc(MAX_FILE_NUM * sizeof(struct file));
        if (f == NULL) {
			uart_send_string("Memory allocation error\n");
            // Handle memory allocation error
        }
    }
}

void traverse_file()
{
	allocate_file_array();
	char* addr = (char *)cpio_addr;
	
   	
	while(utils_string_compare((char *)(addr+sizeof(struct cpio_header)),"TRAILER!!!") == 0){
		
		struct cpio_header *header = (struct cpio_header *)addr;
        unsigned long filename_size = utils_atoi(header->c_namesize, (int)sizeof(header->c_namesize));
        unsigned long headerPathname_size = sizeof(struct cpio_header) + filename_size;
        unsigned long file_size = utils_atoi(header->c_filesize, (int)sizeof(header->c_filesize));
	
		utils_align(&headerPathname_size, 4);
        utils_align(&file_size, 4);
		
		f[file_num].file_header =  header; 
		f[file_num].filename_size = filename_size;
		f[file_num].headerPathname_size = headerPathname_size;
		f[file_num].file_size = file_size;
		f[file_num].file_content_head = (char*) header + headerPathname_size;

		addr += (headerPathname_size + file_size);
		file_num += 1;
	}
}

int findfile(char* filename) {
	for(int n=0;n<=file_num;n++) {
		if ((utils_string_compare(((char *)f[n].file_header)+sizeof(struct cpio_header), filename) != 0)){
           return n;
        }
	}
	return -1;
}

void cpio_ls()
{
	for(int n=0;n<=file_num;n++) {
		uart_send_string(((char *)f[n].file_header)+sizeof(struct cpio_header));
		uart_send_string("\n");
	}
}


void cpio_cat(char *filename)
{
	int targetfile_num = findfile(filename);

	if(targetfile_num != -1) {	
		for (unsigned int i = 0; i < f[targetfile_num].file_size; i++)
        {
            uart_send_char(f[targetfile_num].file_content_head[i]);
        }
        uart_send_string("\n");
		
	} else {
		uart_send_string("Can not Find the file\n");
	}
        
}

void cpio_exec_program(char* filename) {
	int targetfile_num = findfile(filename);

	char* target = (char*) 0x20000;
	for(unsigned i = 0; i<f[targetfile_num].file_size; i++){
		*target++ = f[targetfile_num].file_content_head[i];
	}

	unsigned long spsr_el1 = 0x3c0;
	asm volatile ("msr spsr_el1, %0" ::"r"(spsr_el1));
	unsigned long jump_addr = 0x20000;
	asm volatile("msr elr_el1,%0" ::"r"(jump_addr));
	unsigned long sp = (unsigned long) simple_malloc(4096);
	asm volatile("msr sp_el0,%0" :: "r"(sp));
	asm volatile("eret");
}
