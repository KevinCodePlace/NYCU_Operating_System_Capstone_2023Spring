#pragma once
#include "vfs.h"
#define tid_t unsigned int
#define pid_t unsigned int

int Thread(void *func(void), ...);
void idle();
void init_thread();
void set_first_thread();
void clear_threads();
void handle_child(tid_t tid);
void push_first_thread();
void printf_thread();
void record_mem(void* addr);
int getpid();
int set_fork(void* sp);
int kill(pid_t pid);
int move_last_mem(tid_t tid);

#define thread_numbers 65536

struct thread_sibling{
    struct thread *self;
    struct thread_sibling *next;
};

struct thread{
    unsigned long long registers[2*7];
    struct thread* next;
    unsigned int signal;
    void* sig_handler[32];
    void* malloc_table[256];
    int priority;
    tid_t tid, ptid;
    enum STATUS{
        starting,
        running,
        stop,
        dead
    } status;
    struct thread_sibling *childs;
    unsigned char *ustack;
    unsigned char *kstack;
    void *page_table;
    struct file *fd[65536];
    struct vnode *CurWorkDir;
};

typedef struct thread thread_t;