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

    child->ttbr0 = kcalloc(PAGE_SIZE);
    child->user_stack = alloc_stack(child->ttbr0, STACK_SIZE);
    memcpy(child->user_stack, current->user_stack, STACK_SIZE);

    child->kernel_stack = kcalloc(STACK_SIZE);
    TrapFrame *child_trapframe = (TrapFrame *)((unsigned long)child->kernel_stack + STACK_SIZE - sizeof(TrapFrame));
    memcpy(child_trapframe, _regs, sizeof(TrapFrame));

    child->user_prog = alloc_prog(child->ttbr0, current->user_prog_size, current->user_prog);

    setup_peripheral_identity(child->ttbr0);
    
    child_trapframe->regs[0] = 0; // child process : return 0

    child->cpu_context.sp = (unsigned long)child_trapframe;
    child->cpu_context.lr = (unsigned long)restore_regs_eret;

    return child;
}

size_t do_fork(TrapFrame *_regs)
{
    struct task *child = fork_context(_regs);
    add_task(child);
    return child->pid;
}
