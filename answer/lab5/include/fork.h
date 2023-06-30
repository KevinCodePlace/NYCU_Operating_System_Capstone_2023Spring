#ifndef __FORK_H_
#define __FORK_H_
#include <stddef.h>
#include "trap_frame.h"

size_t do_fork(TrapFrame *regs);

#endif