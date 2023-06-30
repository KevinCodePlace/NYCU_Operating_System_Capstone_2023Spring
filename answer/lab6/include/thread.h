#ifndef _THREAD_H
#define _THREAD_H
#include "list.h"
#include "utils_s.h"
#include "sche.h"
#define STACK_SIZE 0x4000

void thread_kill_zombies();
void thread_schedule(size_t _);
void thread_init();
struct task *thread_create(void *func);
void test_thread();
#endif
