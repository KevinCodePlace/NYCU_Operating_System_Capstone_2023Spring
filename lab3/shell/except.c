#include "header/uart.h"
#include "header/irq.h"
#include "header/shell.h"
#include "header/timer.h"

#define CNTPSIRQ_BIT_POSITION 0x02
#define AUXINIT_BIT_POSTION 1<<29

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

	asm volatile("msr cntp_ctl_el0,%0"::"r"(0));
	// Disable interrupts to protect critical section
	asm volatile("msr DAIFSet, 0xf");

	uint64_t current_time;
	asm volatile("mrs %0, cntpct_el0":"=r"(current_time));

	while(timer_head && timer_head->expiry <= current_time) {
		timer_t *timer = timer_head;

		//Execute the callback
		timer->callback(timer->data);
 
		// Remove timer from the list
        timer_head = timer->next;
        if (timer_head) {
            timer_head->prev = NULL;
        }
		
		//free timer
		
		// Reprogram the hardware timer if there are still timers left
		if(timer_head) {
			asm volatile("msr cntp_cval_el0, %0"::"r"(timer_head->expiry));
			asm volatile("msr cntp_ctl_el0,%0"::"r"(1));
		} else {
			asm volatile("msr cntp_ctl_el0,%0"::"r"(0));
		}
	

		//enable interrupt
		asm volatile("msr DAIFClr,0xf");
	}
	
	/*
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
	*/
}

void uart_irq_handler(){
	//uart_send_string("in uart interruption\n");
	
	 uint32_t iir = mmio_read(AUX_MU_IIR);
    // check if it is a receive interrupt
    if ((iir & 0x06) == 0x04) {
        // read data(8 bytes) and store it in the read buffer
        //uart_send_string("\nin receive interrupt\n");
		char data = mmio_read(AUX_MU_IO) & 0xff;
        uart_read_buffer[uart_read_index++] = data;
        if (uart_read_index >= UART_BUFFER_SIZE) {
            uart_read_index = 0;
        }

        // enqueue the received data into the write buffer
        uart_write_buffer[uart_write_index++] = data;
        if (uart_write_index >= UART_BUFFER_SIZE) {
            uart_write_index = 0;
        }
        // enable tx interrupt
		//uart_send_string("\nenable transmit\n");
        mmio_write(AUX_MU_IER, mmio_read(AUX_MU_IER) | 0x2);
    }

    // check if it is a transmit interrupt
    if ((iir & 0x06) == 0x02) {
		//uart_send_string("\nin transmit interrupt\n");
        
		if (uart_write_buffer[uart_write_index-1] == '\r'){
			uart_write_buffer[uart_write_index++] = '\n';
			uart_write_buffer[uart_write_index] = '\0';
		}

		// send data from the write buffer
        if (uart_write_head != uart_write_index) {
            mmio_write(AUX_MU_IO, uart_write_buffer[uart_write_head++]);
            if (uart_write_index >= UART_BUFFER_SIZE) {
                uart_write_index = 0;
            }
        } else {
			//uart_send_string("disable transmit\n");
            // disable tx interrupt when there is no data left to send
			mmio_write(AUX_MU_IER, mmio_read(AUX_MU_IER) & ~0x2);
			
			if(uart_read_buffer[uart_read_index-1] == '\r'){
				uart_read_buffer[uart_read_index-1] = '\0';
				parse_command(uart_read_buffer);
				uart_read_index = 0;
				uart_write_index = 0;
				uart_write_head = 0;
			}
        }
    }
}

void irq_except_handler_c() {

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


