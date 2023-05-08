#include "header/shell.h"
#include "header/uart.h"
#include "header/utils.h"
#include "header/mailbox.h"
#include "header/reboot.h"
#include "header/cpio.h"
#include "header/allocator.h"
#include "header/dtb.h"
#include "header/irq.h"
#define BUFFER_MAX_SIZE 256u


extern void *_dtb_ptr;

void read_command(char* buffer) {
	int index = 0;
	while(1) {
		buffer[index] = uart_get_char();
		uart_send_char(buffer[index]);
		if(buffer[index] == '\n') {
			buffer[index] = '\0';
			buffer[index+1] = '\n';
			break;
		}
		index++;
	}
}

void parse_command(char* buffer){
	char* input_string = buffer;
	if(utils_string_compare(input_string,"help")) {
       uart_send_string("help	:print this help menu\n");
       uart_send_string("hello	:print Hello World!\n");
       uart_send_string("info	:Get the hardware's information\n");
       uart_send_string("reboot	:reboot the device\n");
	   uart_send_string("ls	:list the file\n");
	   uart_send_string("cat	:print file content\n");
	   uart_send_string("malloc	:give dynamic memory space\n");
	   uart_send_string("dtb	:print device tree\n");
	   uart_send_string("exec	:execute user program\n");
	 } else if (utils_string_compare(input_string,"hello")) {
       uart_send_string("Hello World!\n");
     } else if (utils_string_compare(input_string,"info")) {
           get_board_revision();
           uart_send_string("My board revision is: ");
           uart_hex(mailbox[5]);
           uart_send_string("\n");
           get_arm_mem();
           uart_send_string("My ARM memory base address is: ");
           uart_hex(mailbox[5]);
           uart_send_string("\n");
           uart_send_string("My ARM memory size is: ");
           uart_hex(mailbox[6]);
           uart_send_string("\n");  
     } else if (utils_string_compare(input_string,"reboot")) {
           uart_send_string("Rebooting....\n");
           reset(1000);
     } else if (utils_string_compare(input_string,"ls")) {
	       cpio_ls();
     } else if (utils_string_compare(input_string,"cat")){
		   uart_send_string("Filename: ");
		   char filename[BUFFER_MAX_SIZE];
		   read_command(filename);
		   cpio_cat(filename);
	 } else if (utils_string_compare(input_string,"malloc")){
		 char *a = simple_malloc(sizeof("9876"));
		 char *b = simple_malloc(sizeof("345"));
		 a[0] = '9';
		 a[1] = '8';
		 a[2] = '7';
		 a[3] = '6';
		 a[4] = '\0';
		 b[0] = '3';
		 b[1] = '4';
		 b[2] = '5';
		 b[3] = '\0';
		 uart_send_string(a);
		 uart_send_char('\n');
	     uart_send_string(b);
		 uart_send_char('\n');	 
	 }	else if (utils_string_compare(input_string,"dtb")) {
		 fdt_traverse(print_dtb,_dtb_ptr);
	 }  else if (utils_string_compare(input_string,"exec")) {
		 uart_send_string("Program name: ");
		 char buffer[BUFFER_MAX_SIZE];
		 read_command(buffer);
		 cpio_exec_program(buffer);
	 }	else if (utils_string_compare(input_string,"timer")) {
		 asm volatile("msr cntp_ctl_el0,%0"::"r"(1));
		 unsigned long long cntfrq_el0 = 0;
		 asm volatile("mrs %0,cntfrq_el0":"=r"(cntfrq_el0));
		 unsigned long long wait = cntfrq_el0 * 3;
		 asm volatile("msr cntp_tval_el0,%0"::"r"(wait));
		 unsigned int value = 2;
		 unsigned int* address = (unsigned int*) CORE0_TIMER_IRQ_CTRL;
		 *address = value;

	 }	else {
		 uart_send_string("The instruction ");
		 uart_send_string(input_string);
		 uart_send_string(" is not exist.\n");
	 }
}

void shell(){
	
	uart_enable_interrupt();
	/*	
  while(1) {
	 char buffer[BUFFER_MAX_SIZE];
     uart_send_string("# ");
	 read_command(buffer);
	 parse_command(buffer);
  }*/
	
}
