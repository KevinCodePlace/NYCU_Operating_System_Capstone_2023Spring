#include "utils_s.h"
#include "mini_uart.h"
#include "timer.h"
#include "peripheral/mini_uart.h"
#include "exception_c.h"
#include "current.h"
#include "thread.h"
#include "syscall.h"
#define AUX_IRQ (1 << 29)

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }
void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }
unsigned long disable_irq()
{
    unsigned long flag = read_sysreg(DAIF);
    disable_interrupt();
    return flag;
}
void irq_restore(unsigned long flag)
{
    write_sysreg(DAIF, flag);
}

void default_handler()
{
    unsigned long spsr = read_sysreg(spsr_el1);
    unsigned long elr = read_sysreg(elr_el1);
    unsigned long esr = read_sysreg(esr_el1);
    uart_printf("spsr_el1: %x\n", spsr);
    uart_printf("elr_el1: %x\n", elr);
    uart_printf("esr_el1: %x\n\n", esr);
}

void lower_irq_handler()
{
    unsigned long current_time = get_current_time();
    uart_printf("After booting: %d seconds\n\n", current_time);
    set_expired_time(2);
}

void lower_sync_handler(TrapFrame *_regs)
{
    unsigned long esr = read_sysreg(esr_el1); // cause of that exception
    unsigned int ec = ESR_ELx_EC(esr);
    switch (ec)
    {
    case ESR_ELx_EC_SVC64:
        enable_interrupt();
        syscall_handler(_regs);
        disable_interrupt();
        break;
    case ESR_ELx_EC_DABT_LOW:
        uart_send_string("in Data Abort\n");
        break;
    case ESR_ELx_EC_IABT_LOW:
        uart_send_string("in Instruction  Abort\n");
        break;
    default:
        return;
    }
}

void irq_handler()
{
    unsigned int irq_is_pending = (*IRQ_PENDING_1 & AUX_IRQ);
    unsigned int uart = (*AUX_MU_IIR_REG & 0x1) == 0;
    unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
    if (irq_is_pending && uart)
    {
        uart_handler();
    }
    else if (core_timer)
    {
        timer_handler();
    }
}

void curr_sync_handler()
{
    uart_send_string("!!! in current sync handler !!!\n");
    return;
}

void curr_fiq_handler()
{
    uart_send_string("!!! in current fiq handler !!!\n");
    return;
}

void curr_serr_handler()
{
    uart_send_string("!!! in current serr handler !!!\n");
    return;
}
