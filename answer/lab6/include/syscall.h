#ifndef __SYSCALL_H
#define __SYSCALL_H

#include "trap_frame.h"

#define _STR(x) #x
#define STR(s) _STR(s)

typedef void (*syscall)(TrapFrame *);

enum {
    SYS_GETPID,     //0
    SYS_UART_RECV,
    SYS_UART_WRITE,
    SYS_EXEC,
    SYS_FORK,
    SYS_EXIT,       //5
    SYS_MBOX,
    SYS_KILL_PID,
    SYS_SIGNAL,
    SYS_SIGKILL,
    SYS_SIGRETURN,  //10
    NUM_syscalls
};

extern syscall syscall_table[];
void syscall_handler(TrapFrame *regs);

#endif