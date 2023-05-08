#include "header/uart.h"
#include "header/irq.h"

#define CNTPSIRQ_BIT_POSITION 0x02
#define AUXINIT_BIT_POSTION 1<<29
#define BUFFER_SIZE 1024

extern char uart_read_buffer[BUFFER_SIZE];
extern char uart_write_buffer[BUFFER_SIZE];
extern int uart_read_index;
extern int uart_write_index;
extern int uart_write_head;

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

void timer_irq_handler() {

	uart_send_string("\nIn timer interruption\n");

	unsigned long long cntpct_el0 = 0;
	asm volatile("mrs %0,cntpct_el0":"=r"(cntpct_el0));

	unsigned long long cntfrq_el0 = 0;
	asm volatile("mrs %0,cntfrq_el0":"=r"(cntfrq_el0));

	unsigned long long sec = cntpct_el0 / cntfrq_el0;
	uart_send_string("sec:");
	uart_hex(sec);
	uart_send_string("\n");
   
	// Reload the timer (you may need to adjust the timer duration)	
	unsigned long long wait = cntfrq_el0 * 5;
	asm volatile ("msr cntp_tval_el0, %0"::"r"(wait));

}

void uart_irq_handler(){
	uart_send_string("\nIn uart interruption\n");
	uint32_t iir = mmio_read(AUX_MU_IIR);
	 // Check if it is a receive interrupt
    if ((iir & 0x06) == 0x04) {
        uart_send_string("Receiver Interrupt");
		while(1){

		}
    }
}

void irq_except_handler_c() {

	uart_send_string("In Interrupt\n");
	uint32_t irq_pending1 = mmio_read(IRQ_PENDING_1);
	uint32_t core0_interrupt_source = mmio_read(CORE0_INTERRUPT_SOURCE);	
	
	if (core0_interrupt_source & CNTPSIRQ_BIT_POSITION) {
		timer_irq_handler();
    }

    // Handle UART interrupt
    if (irq_pending1 & AUXINIT_BIT_POSTION) {
        uart_irq_handler();
    }		
}


