#ifndef __SCHE_H
#define __SCHE_H
#include "list.h"
#include "signal.h"
#include "mmu.h"

typedef unsigned long pid_t;
typedef enum
{
    TASK_RUNNING,
    TASK_WAITING,
    TASK_STOPPED,
    TASK_INIT,
} state_t;

struct cpu_context
{
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;
    unsigned long sp;
    unsigned long lr;
};

struct task
{
    struct cpu_context cpu_context;
    char *kernel_stack;
    char *user_stack;
    char *user_prog;
    size_t user_prog_size;
    state_t state;
    pid_t pid;
    unsigned need_resched;
    int exitcode;
    unsigned long timeout;
    list list;
    struct signal *signal;
    struct signal_context *sig_context;
    pd_t *ttbr0;
};

extern list running_queue, waiting_queue, stopped_queue;
void add_task(struct task *t);
void kill_task(struct task *_task, int status);
void restart_task(struct task *_task);
void pause_task(struct task *_task);
void sleep_task(size_t ms);
void free_task(struct task *victim);
struct task *create_task();
void switch_task(struct task *next);
struct task *pick_next_task();
int get_the_cur_count();
struct task *get_task(pid_t target);
extern pid_t task_count;
#endif