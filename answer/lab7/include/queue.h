#pragma once
#include "thread.h"
int schedule();
void push2run_queue(struct thread* thread);
void wakeup_queue(struct thread *t);
void exit();
void remove_from_queue(pid_t pid);
void init_queue();