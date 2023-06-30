#include "signal.h"
#include "sche.h"
#include "current.h"
#include "mm.h"
#include "trap_frame.h"
#include "mini_uart.h"
#include "utils_c.h"

static void sig_ignore(int _)
{
    return;
}
void sigkill_handler(int target)
{
    if (current->pid == target)
    {
        kill_task(current, target);
        return;
    }
    struct task *victim = get_task(target);
    if (victim)
    {
        kill_task(victim, 0);
    }
}
void sig_return(void)
{
    asm volatile(
        "mov x8, 10\n"
        "svc 0\n");
}

void sig_context_update(TrapFrame *_regs, void (*handler)())
{
    struct signal_context *sig_context = kmalloc(sizeof(struct signal_context));
    sig_context->trapframe = kmalloc(sizeof(TrapFrame));
    sig_context->user_stack = kmalloc(STACK_SIZE);
    memcpy(sig_context->trapframe, _regs, sizeof(TrapFrame));

    current->sig_context = sig_context;

    _regs->regs[30] = (unsigned long)&sig_return;
    _regs->pc = (unsigned long)handler;
    _regs->sp = (unsigned long)sig_context->user_stack + STACK_SIZE - 0x10;
}

void sig_context_restore(TrapFrame *_regs)
{
    memcpy(_regs, current->sig_context->trapframe, sizeof(TrapFrame));
}

signal_handler signal_table[] = {
    [0] = &sig_ignore,
    [1] = &sig_ignore,
    [2] = &sig_ignore,
    [3] = &sig_ignore,
    [4] = &sig_ignore,
    [5] = &sig_ignore,
    [6] = &sig_ignore,
    [7] = &sig_ignore,
    [8] = &sig_ignore,
    [SIGKILL] = &sigkill_handler,
};