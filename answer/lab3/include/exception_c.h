#ifndef _EXCEPTION_C_H
#define _EXCEPTION_C_H

typedef void (*task_callback)(void *);

void enable_interrupt();
void disable_interrupt();
void default_handler();
void lower_sync_handler();
void curr_irq_handler();
void curr_sync_handler();

void task_init();
void add_task(task_callback cb,void *arg ,unsigned int priority);
void exec_task();

typedef struct task
{
    unsigned long priority;
    unsigned long duration;
    void *arg;
    task_callback callback;
    char msg[20];
    struct task *prev, *next;
} task;

#endif
