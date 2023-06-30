#ifndef __TRAP_FRAME_H
#define __TRAP_FRAME_H

typedef struct
{
    unsigned long regs[31]; // general purpose regs x0~x30
    unsigned long sp;       // sp_el0
    unsigned long pc;       // elr_el1
    unsigned long pstate;   // spsr_el1
}TrapFrame;

#endif