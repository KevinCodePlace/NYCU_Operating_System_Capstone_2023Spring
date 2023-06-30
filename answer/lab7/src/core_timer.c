#include "mini_uart.h"
#include "uint.h"
#include "priority_queue.h"
#include "queue.h"
#include "scheduler.h"
#include "task.h"

uint64 freq_thread = 31;

void core_timer_init(){
    uint64 tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
    core_timer_disable();
}

void arm_core_timer_intr_handler() {
    core_timer_handler();
}

void core_timer_handler(){
    struct node* node = delete_first_node();
    if(node->next == NULL){
        asm volatile("mrs x0, cntfrq_el0\n"
                     "ldr x1, =10\n"
                     "mul x0, x0, x1\n"
                     "msr cntp_tval_el0, x0\n");
        core_timer_disable();
    }else{
        core_timer_enable();
        uint64 interval = node->next->time_to_ring;
        asm volatile("msr cntp_tval_el0, %[output0]\n"
                     ::[output0] "r" (interval));
    }
    free(node);
    node->todo(node->arguments);
}

void add_timer(void (*callback_f)(void*),void *argu_for_call,int times){
    uint64 clock_hz,now_time,interval;
    asm volatile("mrs %[input0], cntfrq_el0\n"
                 "mrs %[input2], cntp_tval_el0\n"
                 :[input0] "=r" (clock_hz),
                  [input2] "=r" (interval));
    uint64 time_to_ring = add_node(callback_f, argu_for_call, clock_hz / 1000 * times, interval);
    core_timer_enable();
    asm volatile("msr cntp_tval_el0, %[output0]\n"
                 ::[output0] "r" (time_to_ring));
}

void *wakeup(void *p){
    uart_printf("Timeout!!\n");
}

void sleep(int duration){
    add_timer(wakeup,NULL,duration);
    uart_printf("Timer is set...\n");
}

void delay(int duration){
    irq_disable();
    void *argu = get_current();
    add_timer(wakeup_queue,argu,duration);
    irq_enable();
    schedule();
}

void thread_timer_handler(){
    void *t = get_current();
    struct thread *s = t;
    PushToReadyList(s->tid);
    thread_timer();
    task_schedule(t);
}

void thread_timer(){
    while(delete_first_node() != NULL);
    add_timer(thread_timer_handler,NULL,freq_thread);
}
