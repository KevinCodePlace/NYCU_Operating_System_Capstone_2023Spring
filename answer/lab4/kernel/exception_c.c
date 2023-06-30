#include "utils_s.h"
#include "mini_uart.h"
#include "timer.h"
#include "peripheral/mini_uart.h"
#include "exception_c.h"
#define AUX_IRQ (1 << 29)

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }
void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }

void default_handler()
{
    disable_interrupt();
    unsigned long spsr = read_sysreg(spsr_el1);
    unsigned long elr = read_sysreg(elr_el1);
    unsigned long esr = read_sysreg(esr_el1);
    uart_printf("spsr_el1: %x\n", spsr);
    uart_printf("elr_el1: %x\n", elr);
    uart_printf("esr_el1: %x\n\n", esr);
    enable_interrupt();
}

void lower_irq_handler()
{
    unsigned long current_time = get_current_time();
    uart_printf("After booting: %d seconds\n\n", current_time);
    set_expired_time(2);
}

void lower_sync_handler()
{
    disable_interrupt();
    unsigned long spsr = read_sysreg(spsr_el1);
    unsigned long elr = read_sysreg(elr_el1);
    unsigned long esr = read_sysreg(esr_el1);
    uart_printf("spsr_el1: %x\n", spsr);
    uart_printf("elr_el1: %x\n", elr);
    uart_printf("esr_el1: %x\n\n", esr);
    enable_interrupt();
}

void curr_irq_handler()
{
    disable_interrupt();
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

void curr_sync_handler(unsigned long esr_el1, unsigned long elr_el1)
{
    disable_interrupt();
    return;
    enable_interrupt();
}

/*
lab3 advance2 dead body
int doing_task = 0;
task *task_queue_head = 0, *task_queue_tail = 0;

void add_task(task_callback cb, void *arg, unsigned int priority)
{
    task *new_task = (task *)smalloc(sizeof(task));
    new_task->priority = priority;
    new_task->callback = cb;
    new_task->arg = arg;
    new_task->next = 0;
    new_task->prev = 0;
    if (task_queue_head == 0)
    {
        task_queue_head = new_task;
        task_queue_tail = new_task;
    }
    else
    {
        task *cur = task_queue_head;
        while (cur)
        {
            if (cur->priority < new_task->priority)
                break;
            cur = cur->next;
        }
        if (cur == 0)
        { // cur at end
            new_task->prev = task_queue_tail;
            task_queue_tail->next = new_task;
            task_queue_tail = new_task;
        }
        else if (cur->prev == 0)
        { // cur at head
            new_task->next = cur;
            (task_queue_head)->prev = new_task;
            task_queue_head = new_task;
        }
        else
        { // cur at middle
            new_task->next = cur;
            new_task->prev = cur->prev;
            (cur->prev)->next = new_task;
            cur->prev = new_task;
        }
    }
}

void exec_task()
{
    while (1)
    {
        task_queue_head->callback(task_queue_head->arg);
        disable_interrupt();
        task_queue_head = task_queue_head->next;
        if (task_queue_head)
        {
            task_queue_head->prev = 0;
        }
        else
        {
            task_queue_head = task_queue_tail = 0;
            return;
        }
        enable_interrupt();
    }
}

void curr_irq_handler_decouple()
{
    unsigned int uart = (*IRQ_PENDING_1 & AUX_IRQ);
    unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
    if (uart)
    {
        Reg *reg = (Reg *)smalloc(sizeof(Reg));
        *reg = *AUX_MU_IER_REG;
        add_task(uart_handler, reg, 3);
        *AUX_MU_IER_REG &= ~(0x3);
    }
    else if (core_timer)
    {
        add_task(timer_handler, NULL, 0);
        core_timer_disable();
    }
    if (!doing_task)
    {
        doing_task = 1;
        enable_interrupt();
        exec_task();
        enable_interrupt();
        doing_task = 0;
    }
}
*/