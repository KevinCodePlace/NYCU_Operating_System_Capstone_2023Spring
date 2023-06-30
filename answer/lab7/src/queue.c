#include "uint.h"
#include "thread.h"
#include "scheduler.h"

struct thread* run_queue = NULL;
extern uint64 freq_thread;
void init_queue(){
    run_queue = NULL;
}

int schedule(){
start:
    if(run_queue == NULL)
        push_first_thread();
    struct thread* prev = get_current();
    struct thread* tmp = run_queue;
    run_queue = run_queue->next;
    
    if(tmp->status == running){
        switch_to(prev,tmp);
    }else if(tmp->status == dead){
        remove_from_queue(tmp->tid);
        goto start;
    }else{
        tmp->status = running;
        if(tmp->tid == 0){
            set_current(tmp);
        }else{
            store_and_jump(prev,tmp);
        }
    }
}

void push2run_queue(struct thread* thread){
    if(run_queue != NULL){
        struct thread *tmp = run_queue;
        while(tmp->next != NULL){
            tmp = tmp->next;
        }
        tmp->next = thread;
    }else{
        run_queue = thread;
    }
    thread->next = NULL;
}

void push2run_queue_top(struct thread* thread){
    thread->next = run_queue;
    run_queue = thread;
}

void wakeup_queue(struct thread *t){
    if(t->status == dead) return;
    t->status = running;
    push2run_queue(t);
}

void exit(){
    struct thread *t = get_current();
    free_mem_table(t);
    t->status = dead;
    schedule();
}

void remove_from_queue(pid_t pid){
    struct thread *t = run_queue;
    if(t != NULL && t->tid == pid) run_queue = run_queue->next;
    else if(t != NULL){
        while(t->next != NULL){
            if(t->next->tid == pid){
                t->next = t->next->next;
                return;
            }
        }
    }
}

