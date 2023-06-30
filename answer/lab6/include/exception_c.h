#ifndef _EXCEPTION_C_H
#define _EXCEPTION_C_H
#include <stddef.h>
#include "trap_frame.h"
typedef void (*task_callback)(void *);

void enable_interrupt();
void disable_interrupt();
unsigned long disable_irq();
void irq_restore(unsigned long flag);

void default_handler();
void lower_sync_handler(TrapFrame *_regs);
void irq_handler();
void curr_sync_handler();

#endif
