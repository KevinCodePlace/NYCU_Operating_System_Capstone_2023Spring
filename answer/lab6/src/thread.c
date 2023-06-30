#include "thread.h"
#include "mini_uart.h"
#include "timer.h"
#include "sche.h"
#include "list.h"
#include "mm.h"
#include "exception_c.h"
#include "current.h"
#include "trap_frame.h"

void thread_kill_zombies()
{
    unsigned long flags = disable_irq();
    while (!list_empty(&stopped_queue))
    {
        struct task *victim = list_first_entry(&stopped_queue, struct task, list);
        unlink(&victim->list);
        free_task(victim);
    }
    irq_restore(flags);
}

void thread_schedule(size_t _)
{
    if (!current->need_resched)
    {
        return;
    }

    unsigned long flags = disable_irq();
    struct task *next = pick_next_task();
    next->need_resched = 0;
    irq_restore(flags);

    switch_task(next);
}

void thread_init()
{
    struct task *init = create_task();
    set_thread_ds(init); // init the thread structure
}

static void foo()
{
    for (int i = 0; i < 5; ++i)
    {
        uart_printf("Thread id: %d i=%d\n", current->pid, i);
        // sleep_task(500);
        delay(1000000);
    }
    kill_task(current, 0);
    thread_schedule(0);
}

struct task *thread_create(void *func)
{
    struct task *new_thread = create_task();
    new_thread->kernel_stack = kmalloc(STACK_SIZE);
    new_thread->state = TASK_RUNNING;
    new_thread->cpu_context.lr = (unsigned long)func;
    new_thread->cpu_context.sp = (unsigned long)new_thread->kernel_stack + STACK_SIZE - sizeof(TrapFrame);

    add_task(new_thread);

    return new_thread;
}

void test_thread()
{
    for (int i = 0; i < 3; ++i)
    {
        thread_create(&foo);
    }
    uart_printf("end of test_thread\n");
}
