#include "syscall.h"
#include "stddef.h"
#include "trap_frame.h"
#include "current.h"
#include "peripheral/mailbox.h"
#include "mini_uart.h"
#include "signal.h"
#include "exec.h"
#include "exception_c.h"
#include "mm.h"
#include "fork.h"

void sys_getpid(TrapFrame *_regs)
{
    _regs->regs[0] = current->pid;
}
void sys_uartrecv(TrapFrame *_regs)
{
    char *buf = (char *)_regs->regs[0];
    int count = _regs->regs[1];
    for (int i = 0; i < count; i++)
    {
        buf[i] = uart_recv();
    }
    _regs->regs[0] = count;
}
void sys_uartwrite(TrapFrame *_regs)
{
    char *buf = (char *)_regs->regs[0];
    int count = _regs->regs[1];
    for (int i = 0; i < count; i++)
    {
        uart_send(buf[i]);
    }
    _regs->regs[0] = count;
}
void sys_exec(TrapFrame *_regs)
{
    const char *path = (char *)_regs->regs[0];
    const char **args = (const char **)_regs->regs[1];
    _regs->regs[0] = do_exec(path, args);
}
void sys_fork(TrapFrame *_regs)
{
    _regs->regs[0] = do_fork(_regs);
}
void sys_exit(TrapFrame *_regs)
{
    kill_task(current, _regs->regs[0]);
}
void sys_mbox_call(TrapFrame *_regs)
{
    unsigned int channel = _regs->regs[0];
    unsigned int *mailbox_va = (unsigned int *)_regs->regs[1];

    void *mailbox_pa = (unsigned int *)el0_va2pa((unsigned long)mailbox_va);
    mailbox_call(channel, mailbox_pa);
}
void sys_kill_pid(TrapFrame *_regs)
{
    pid_t target = _regs->regs[0];
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
void sys_signal(TrapFrame *_regs)
{
    int sig_num = _regs->regs[0];
    signal_handler _hand = (signal_handler)_regs->regs[1];

    struct signal *new_signal = kmalloc(sizeof(struct signal));
    new_signal->sig_num = sig_num;
    new_signal->handler = _hand;
    new_signal->list.next = new_signal->list.prev = &new_signal->list;

    if (!current->signal)
    {
        current->signal = new_signal;
    }
    else
    {
        insert_tail(&current->signal->list, &new_signal->list);
    }
}

void sys_sigkill(TrapFrame *_regs)
{
    int target = _regs->regs[0];
    int SIGNAL = _regs->regs[1];
    int is_find = 0;
    if (current->signal)
    {
        struct signal *cur = current->signal;
        do
        {
            if (cur->sig_num == SIGNAL)
            {
                is_find = 1;
                sig_context_update(_regs, cur->handler);
                break;
            }
            cur = list_entry(cur->list.next, struct signal, list);
        } while (cur != current->signal);
    }
    else if (!current->signal && !is_find)
    {
        (signal_table[SIGNAL])(target);
    }
}
void sys_sigreturn(TrapFrame *_regs)
{
    sig_context_restore(_regs);

    disable_interrupt();
    kfree(current->sig_context->trapframe);
    kfree(current->sig_context->user_stack);
    kfree(current->sig_context);
    current->sig_context = NULL;
    enable_interrupt();
}
syscall syscall_table[NUM_syscalls] = {
    [SYS_GETPID] = &sys_getpid,
    [SYS_UART_RECV] = &sys_uartrecv,
    [SYS_UART_WRITE] = &sys_uartwrite,
    [SYS_EXEC] = &sys_exec,
    [SYS_FORK] = &sys_fork,
    [SYS_EXIT] = &sys_exit,
    [SYS_MBOX] = &sys_mbox_call,
    [SYS_KILL_PID] = &sys_kill_pid,
    [SYS_SIGNAL] = &sys_signal,
    [SYS_SIGKILL] = &sys_sigkill,
    [SYS_SIGRETURN] = &sys_sigreturn,
};

void syscall_handler(TrapFrame *_regs)
{
    unsigned int sys_index = _regs->regs[8];
    if (sys_index >= NUM_syscalls)
    {
        uart_send_string("!!! Invalid system call !!!\n");
        return;
    }
    (syscall_table[sys_index])(_regs);
}
