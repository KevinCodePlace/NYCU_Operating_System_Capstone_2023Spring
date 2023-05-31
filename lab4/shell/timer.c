#include "header/timer.h"
#include "header/allocator.h"
#include "header/uart.h"
#include "header/irq.h"
#include "header/utils.h"

timer_t *timer_head = NULL;

void add_timer(timer_t *new_timer) {

    timer_t *current = timer_head;

    // Disable interrupts to protect the critical section
    asm volatile("msr DAIFSet, 0xf");
    // Special case: the list is empty or the new timer is the earliest
    if (!timer_head || new_timer->expiry < timer_head->expiry) {
        new_timer->next = timer_head;
        new_timer->prev = NULL;
        if (timer_head) {
            timer_head->prev = new_timer;
        }
        timer_head = new_timer;

        // Reprogram the hardware timer
        asm volatile ("msr cntp_cval_el0, %0"::"r"(new_timer->expiry));
		asm volatile("msr cntp_ctl_el0,%0"::"r"(1));

        // Enable interrupts
        asm volatile("msr DAIFClr, 0xf");
        return;
    }

    // Find the correct position in the list
    while (current->next && current->next->expiry < new_timer->expiry) {
        current = current->next;
    }

    // Insert the new timer
    new_timer->next = current->next;
    new_timer->prev = current;
    if (current->next) {
        current->next->prev = new_timer;
    }
    current->next = new_timer;
	// Enable interrupts
    asm volatile("msr DAIFClr, 0xf");
}


void create_timer(timer_callback callback, void* data, uint64_t after) {
	//Allocate memory for the timer
	timer_t* timer = simple_malloc(sizeof(timer_t));
	if(!timer) {
		return;
	}

	//Set the callback and data
	timer->callback = callback;
	timer->data = data;

	//Calculate the expiry time
	uint64_t current_time, cntfrq;
	asm volatile("mrs %0, cntpct_el0":"=r"(current_time));
	asm volatile("mrs %0, cntfrq_el0":"=r"(cntfrq));
	timer->expiry = current_time + after * cntfrq;
	//Add the time to the list
	add_timer(timer);
}


void print_message(void *data) {
	char *message = data;
	uint64_t current_time, cntfrq;
    asm volatile("mrs %0, cntpct_el0" : "=r"(current_time));
    asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq));
    uint64_t seconds = current_time / cntfrq;

	uart_send_string("Timeout message: ");
	uart_send_string(message);
	uart_send_string(" occurs at ");
	uart_hex(seconds);
	uart_send_string("\n");
}

void setTimeout(char *message,uint64_t seconds) {
	
	char *message_copy = utils_strdup(message);

	if(!message_copy){
		return;
	}

	if (!timer_head) {
		//enable core_timer_interrupt
		unsigned int value = 2;
		unsigned int* address = (unsigned int*) CORE0_TIMER_IRQ_CTRL;
		*address = value;
	}
	
	create_timer(print_message,message_copy,seconds);
}

