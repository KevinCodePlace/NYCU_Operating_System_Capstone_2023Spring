#ifndef __SIGNAL_H
#define __SIGNAL_H

#define SIGKILL 9

#include "list.h"
#include "trap_frame.h"
#define SIG_NUM (sizeof(signal_table) / sizeof(signal_table[0]))
typedef void (*signal_handler)(int);

struct signal
{
    unsigned int sig_num;
    signal_handler handler;
    struct list list;
};
struct signal_context
{
    TrapFrame *trapframe;
    char *user_stack;
};

extern signal_handler signal_table[];

void sig_context_update(TrapFrame *_regs, void (*handler)());
void sig_context_restore(TrapFrame *_regs);
#endif
