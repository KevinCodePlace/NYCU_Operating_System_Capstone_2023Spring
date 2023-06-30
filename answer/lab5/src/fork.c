#include "fork.h"
#include "stddef.h"
#include "sche.h"
#include "thread.h"
#include "exception_c.h"
#include "exception_s.h"
#include "utils_c.h"
#include "current.h"
#include "mm.h"

static struct task *fork_context(TrapFrame *_regs)
{
    struct task *child = kmalloc(sizeof(struct task));

    unsigned long flags = disable_irq();
    *child = *current; // copy the current to child entirely
    child->pid = task_count++;
    irq_restore(flags);

    child->need_resched = 0;

    child->user_stack = kmalloc(STACK_SIZE);
    memcpy(child->user_stack, current->user_stack, STACK_SIZE);

    child->kernel_stack = kmalloc(STACK_SIZE);
    TrapFrame *trapframe = (TrapFrame *)((unsigned long)child->kernel_stack + STACK_SIZE - sizeof(TrapFrame));
    memcpy(trapframe, _regs, sizeof(TrapFrame));

    child->user_prog = kmalloc(current->user_prog_size);
    memcpy(child->user_prog, current->user_prog, current->user_prog_size);

    trapframe->regs[30] = (unsigned long)child->user_prog + (_regs->regs[30] - (unsigned long)current->user_prog); // using x30 as link return register while function call on AArch64
    trapframe->sp = (unsigned long)child->user_stack + (_regs->sp - (unsigned long)current->user_stack);
    trapframe->pc = (unsigned long)child->user_prog + (_regs->pc - (unsigned long)current->user_prog);
    trapframe->regs[0] = 0; // child process : return 0

    child->cpu_context.sp = (unsigned long)trapframe;
    child->cpu_context.lr = (unsigned long)restore_regs_eret;
    return child;
}

size_t do_fork(TrapFrame *_regs)
{
    struct task *child = fork_context(_regs);
    add_task(child);
    return child->pid;
}
