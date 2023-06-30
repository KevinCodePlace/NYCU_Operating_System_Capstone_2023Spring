#include "uint.h"
#include "mini_uart.h"
#include "peripheral/irq.h"
#include "aux.h"
#include "timer.h"
#include "gpio.h"
#include "interrupt_queue.h"

void irq_init_vectors(){
    asm volatile("ldr     x0, =exception_table\n"
                 "msr     vbar_el1, x0\n");
    return;
}

void enable_interrupt_controller() {
    uint32 *tmp = irq0_enable_1;
    *tmp |= (1 << 29);
}

int handle_irq() {
    uint64 *core_irq=0xFFFF000040000060;
    uint32 *irq;
    irq = irq0_pending_1;
    if (*irq & (1 << 29)) {
        *irq &= ~(1 << 29);
        *AUX_MU_IER = 0;
        push_queue(handle_uart_irq);
        return 1;
    }
    if ((*core_irq) & 2) {
        *irq &= ~2;
        core_timer_disable();
        push_queue(arm_core_timer_intr_handler);
        return 1;
    }
    return 0;
}
