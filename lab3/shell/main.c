#include "header/uart.h"
#include "header/shell.h"
#include "header/dtb.h"
#include "header/utils.h"
#include "header/cpio.h"

extern void *_dtb_ptr;
void main()
{

    // set up serial console
    uart_init();

	unsigned long el = 0;
	asm volatile ("mrs %0, CurrentEL":"=r"(el));
	uart_send_string("Current exception level: ");
	uart_hex(el>>2); // CurrentEL store el level at [3:2]
	uart_send_string("\n");

	asm volatile("mov %0, sp"::"r"(el));
	uart_send_string("Current stack pointer address: ");
	uart_hex(el);
	uart_send_string("\n");
	
	// say hello
	fdt_traverse(get_cpio_addr,_dtb_ptr);
    traverse_file();
	uart_send_string("Type in `help` to get instruction menu!\n");
    
	//echo everything back
	shell();
}


void except_handler_c() {
	uart_send_string("In Exception handle\n");

	//read spsr_el1
	unsigned long long spsr_el1 = 0;
	asm volatile("mrs %0, spsr_el1":"=r"(spsr_el1));
	uart_send_string("spsr_el1: ");
	uart_hex(spsr_el1);
	uart_send_string("\n");

	//read elr_el1
	unsigned long long elr_el1 = 0;
	asm volatile("mrs %0, elr_el1":"=r"(elr_el1));
	uart_send_string("elr_el1: ");
	uart_hex(elr_el1);
	uart_send_string("\n");
	
	//esr_el1
	unsigned long long esr_el1 = 0;
	asm volatile("mrs %0, esr_el1":"=r"(esr_el1));
	uart_hex(esr_el1);
	uart_send_string("\n");

	//ec
	unsigned ec = (esr_el1 >> 26) & 0x3F; //0x3F = 0b111111(6)
	uart_send_string("ec: ");
	uart_hex(ec);
	uart_send_string("\n");

	while(1){

	}
}

void irq_except_handler_c() {
	
	uart_send_string("In timer interruption\n");

	unsigned long long cntpct_el0 = 0;
	asm volatile("mrs %0,cntpct_el0":"=r"(cntpct_el0));
	
	unsigned long long cntfrq_el0 = 0;
	asm volatile("mrs %0,cntfrq_el0":"=r"(cntfrq_el0));

	unsigned long long sec = cntpct_el0 / cntfrq_el0;
	uart_send_string("sec:");
	uart_hex(sec);
	uart_send_string("\n");
	
	unsigned long long wait = cntfrq_el0 * 2;
	asm volatile ("msr cntp_tval_el0, %0"::"r"(wait));
	
}

/*
void except_handle() {
	uart_send_string("In exception");
	
	unsigned long long esr_el1 = 0;
    asm volatile ("mrs %0, esr_el1" :: "r" (esr_el1));
    unsigned ec = (esr_el1 >> 26) & 0x3F; //0x3F = 0b111111(6)
    unsigned imm16 = esr_el1 & 65535; //65535 = 0x1111111111111111(16)

	uart_send_string("\nec: ");
    uart_hex(ec);
    uart_send_string("\n");
    uart_send_string("imm16: ");
    uart_hex(imm16);
    uart_send_string("\n");

    if (ec != 0x15){ // SVC from AArch64
		while(1) nop;
	}
    
    switch (imm16) {
		case 0x1337: { // exit from EL0
			// update spsr to make eret jumps back to loop
			// set EL1h and mask all interrupts
			uart_send_string("In the svc execution");	
			return;
		}
    }
}
*/
