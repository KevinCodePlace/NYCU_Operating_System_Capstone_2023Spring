#include "timer.h"
#include "utils_s.h"
#include "allocator.h"
#include "mini_uart.h"
#include "utils_c.h"
timeout_event *timeout_queue_head, *timeout_queue_tail;

void core_timer_enable()
{
    /*
        cntpct_el0 >= cntp_cval_el0 -> interrupt
        cntp_tval_el0 = cntpct_el0 - cntp_cval_el0
    */
    write_sysreg(cntp_ctl_el0, 1); // enable
    unsigned long frq = read_sysreg(cntfrq_el0);
    write_sysreg(cntp_tval_el0, frq * 2); // set expired time
    *CORE0_TIMER_IRQ_CTRL = 2;            // unmask timer interrupt
}

void core_timer_disable()
{
    write_sysreg(cntp_ctl_el0, 0); // disable
    *CORE0_TIMER_IRQ_CTRL = 0;     // unmask timer interrupt
}

void set_expired_time(unsigned long duration)
{
    unsigned long frq = read_sysreg(cntfrq_el0);
    write_sysreg(cntp_tval_el0, frq * duration); // set expired time
}

unsigned long get_current_time()
{
    // cntpct_el0: The timer’s current count.
    unsigned long frq = read_sysreg(cntfrq_el0);
    unsigned long current_count = read_sysreg(cntpct_el0);
    return (unsigned long)(current_count / frq);
}

void set_timeout(char *message, char *_time)
{
    unsigned int time = utils_str2uint_dec(_time);
    add_timer(print_message, message, time);
}

void print_message(char *msg)
{
    uart_send_string(msg);
}

void timeout_event_init()
{
    timeout_queue_head = 0;
    timeout_queue_tail = 0;
}

void add_timer(timer_callback cb, char *msg, unsigned long duration)
{
    timeout_event *new_event = (timeout_event *)malloc(sizeof(timeout_event));
    new_event->register_time = get_current_time();
    new_event->duration = duration;
    new_event->callback = cb;
    for (int i = 0; i < 20; i++)
    {
        new_event->msg[i] = msg[i];
        if (msg[i] == '\0')
            break;
    }
    new_event->next = 0;
    new_event->prev = 0;

    if (timeout_queue_head == 0)
    {
        timeout_queue_head = new_event;
        timeout_queue_tail = new_event;
        core_timer_enable();
        set_expired_time(duration);
    }
    else
    {
        unsigned long timeout= new_event->register_time + new_event->duration;
        timeout_event *cur = timeout_queue_head;
        while (cur)
        {
            if ( (cur->register_time + cur->duration) > timeout)
                break;
            cur = cur->next;
        }
        if (cur == 0)
        { // cur at end
            new_event->prev = timeout_queue_tail;
            timeout_queue_tail->next = new_event;
            timeout_queue_tail = new_event;
        }
        else if (cur->prev == 0)
        { // cur at head
            new_event->next = cur;
            (timeout_queue_head)->prev = new_event;
            timeout_queue_head = new_event;
            set_expired_time(duration);
        }
        else
        { // cur at middle
            new_event->next = cur;
            new_event->prev = cur->prev;
            (cur->prev)->next = new_event;
            cur->prev = new_event;
        }
    }
}

void timer_handler(void *arg)
{
   unsigned long current_time = get_current_time();
    uart_send_string("\nmessage :");
    timeout_queue_head->callback(timeout_queue_head->msg);
    uart_printf("\ncurrent time : %ds\n", current_time);
    uart_printf("command executed time : %ds\n", timeout_queue_head->register_time);
    uart_printf("command duration time : %ds\n\n", timeout_queue_head->duration);

    timeout_event *next = timeout_queue_head->next;
    if (next)
    {
       next->prev = 0;
        timeout_queue_head = next;
        core_timer_enable();
        set_expired_time(next->register_time + next->duration - get_current_time());
    }
    else // no other event
    {
        timeout_queue_head = timeout_queue_tail = 0;
        core_timer_disable();
    }
}