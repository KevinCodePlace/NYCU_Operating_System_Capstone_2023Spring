#include "header/shell.h"
#include "header/uart.h"
#include "header/utils.h"
#include "header/mailbox.h"
#include "header/reboot.h"
#include "header/cpio.h"
#define BUFFER_MAX_SIZE 256u


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

void shell(){
  //char array_space[256];
  //char* input_string = array_space;
  char buffer[BUFFER_MAX_SIZE];
  while(1) {
     uart_send_string("# ");
	 read_command(buffer);
     char * input_string = buffer;
     if(utils_string_compare(input_string,"help")) {
       uart_send_string("help	:print this help menu\n");
       uart_send_string("hello	:print Hello World!\n");
       uart_send_string("info	:Get the hardware's information\n");
       uart_send_string("reboot	:reboot the device\n");
	   uart_send_string("ls	:list the file\n");
	   uart_send_string("cat	:print file content\n");
     } else if (utils_string_compare(input_string,"hello")) {
       uart_send_string("Hello World!\n");
     } else if (utils_string_compare(input_string,"info")) {
           get_board_revision();
           uart_send_string("My board revision is: ");
           uart_binary_to_hex(mailbox[5]);
           uart_send_string("\n");
           get_arm_mem();
           uart_send_string("My ARM memory base address is: ");
           uart_binary_to_hex(mailbox[5]);
           uart_send_string("\n");
           uart_send_string("My ARM memory size is: ");
           uart_binary_to_hex(mailbox[6]);
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
	 } else {
		 uart_send_string("The instruct is not exist.\n");
	 }
  }
}
