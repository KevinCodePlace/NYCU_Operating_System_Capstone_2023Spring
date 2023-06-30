#include "sche.h"
#include "current.h"
#include "timer.h"
#include "exception_c.h"
#include "mini_uart.h"
#include "mm.h"

pid_t task_count = 0;

list running_queue = LIST_HEAD_INIT(running_queue);
list waiting_queue = LIST_HEAD_INIT(waiting_queue);
list stopped_queue = LIST_HEAD_INIT(stopped_queue);

int get_the_cur_count()
{
    int count = 0;
    list *head = &running_queue;
    while (head->next != &running_queue)
    {
        count++;
        head = head->next;
    }
    return count;
}

void add_task(struct task *t)
{
    size_t flags = disable_irq();
    insert_tail(&running_queue, &t->list);
    irq_restore(flags);
}

void kill_task(struct task *_task, int status)
{
    size_t flags = disable_irq();

    _task->state = TASK_STOPPED;
    _task->need_resched = 1;
    unlink(&_task->list);
    _task->exitcode = status;
    insert_head(&stopped_queue, &_task->list);

    irq_restore(flags);
    thread_schedule(0);
}

void restart_task(struct task *_task)
{
    size_t flags = disable_irq();

    if (_task->state != TASK_WAITING)
    {
        uart_send_string("---- task state inconsistent ----\n");
        return;
    }
    _task->state = TASK_RUNNING;
    unlink(&_task->list);
    insert_tail(&running_queue, &_task->list);

    irq_restore(flags);
}

void pause_task(struct task *_task)
{
    size_t flags = disable_irq();

    if (_task->state != TASK_RUNNING)
    {
        uart_send_string("---- task state inconsistent ----\n");
        return;
    }
    _task->state = TASK_WAITING;
    _task->need_resched = 1;
    unlink(&_task->list);
    insert_head(&waiting_queue, &_task->list);

    irq_restore(flags);
}

void sleep_task(size_t ms)
{
    size_t flags = disable_irq();

    add_timer((timer_callback)restart_task, (size_t)current, ms);
    pause_task(current);
    irq_restore(flags);

    thread_schedule(0);
}

void free_task(struct task *victim)
{
    if (victim->kernel_stack)
        kfree(victim->kernel_stack);
    kfree(victim);
}

struct task *create_task()
{
    struct task *new_task = kmalloc(sizeof(struct task));
    new_task->cpu_context.lr = new_task->cpu_context.sp = new_task->cpu_context.fp = 0;
    new_task->kernel_stack = NULL;
    new_task->user_stack = NULL;
    new_task->user_prog = NULL;
    new_task->user_prog_size = 0;
    new_task->state = TASK_INIT;
    new_task->pid = task_count++;
    new_task->need_resched = 0;
    new_task->exitcode = 0;
    new_task->timeout = get_current_time() + DEFAULT_TIMEOUT;
    new_task->signal = NULL;
    new_task->sig_context = NULL;
    new_task->ttbr0 = NULL;
    return new_task;
}

struct task *pick_next_task()
{
    if (list_empty(&running_queue))
    {
        while (1)
        {
        };
    }
    struct task *next_task = list_first_entry(&running_queue, struct task, list);
    unlink(&next_task->list);
    insert_tail(&running_queue, &next_task->list);

    return next_task;
}

void switch_task(struct task *next)
{
    if (current == next)
    {
        return;
    }
    switch_to(&current->cpu_context, &next->cpu_context);
    replace_page_table();
}

struct task *get_task(pid_t target)
{
    struct task *_task;
    list_for_each_entry(_task, &running_queue, list)
    {
        if (_task->pid == target)
        {
            return _task;
        }
    }
    return NULL;
}