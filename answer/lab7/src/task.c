#include "thread.h"
#include "uint.h"
#include "queue.h"
#include "task.h"
#include "scheduler.h"
#include "allocator.h"
#include "excep.h"
#include "mmu.h"
#include "cpio.h"
#include "string.h"

struct thread* ReadyList = NULL;
extern struct thread *threads[thread_numbers];

void UserScheduler(){
    if(ReadyList == NULL){
        core_timer_disable();
        while(delete_first_node() != NULL);
        exit();
    }else{
        struct thread *t = ReadyListPop();
        set_ttbr0_el1(t->page_table);
        SwitchTo(t);
    }
}

void UserExit(){
    struct thread *t = get_current();
    t->status = dead;
    UserScheduler();
}

void* ReadyListPop(){
    void * tmp = ReadyList;
    ReadyList = ReadyList->next;
    return tmp;
}

void InitUserTaskScheduler(){
    thread_timer();
    struct thread *t = get_current();
    t->status = dead;
    UserScheduler();
}

void PushToReadyList(pid_t pid){
    struct thread* item = threads[pid];
    struct thread *tmp = ReadyList;
    if(tmp == NULL){
        ReadyList = item;
    }else{
        while(tmp->next != NULL) tmp = tmp->next;
        tmp->next = item;
    }
    item->next = NULL;
}

void RemoveItemFromReadyList(pid_t pid){
    struct thread *tmp = ReadyList;
    if(tmp != NULL){
        if(tmp->tid == pid){
            ReadyList = ReadyList->next;
        }else{
            while(tmp->next != NULL){
                if(tmp->next->tid == pid){
                    tmp->next = tmp->next->next;
                }
            }
        }
    }
}

int UserKill(pid_t pid){
    threads[pid]->status = dead;
    RemoveItemFromReadyList(pid);
}

int UserThread(void* func,void* arg){
    struct thread *t;
    t = malloc(sizeof(struct thread));
    for(int i=0;i<thread_numbers;i++){
        if(threads[i] == NULL){
            threads[i] = t;
            t->tid = i;
            break;
        }
    }
    delete_last_mem();
    unsigned char* kstack = malloc(0x10000);
    t->next = NULL;
    for(int i=0;i<32;i++) t->sig_handler[i] = NULL;
    t->sig_handler[9] = call_exit;
    t->sig_handler[10] = UserKill;
    t->page_table = ((uint64_t)allocate_page() | 0b11) &  ~0xffff000000000000;
    t->signal = 0;
    t->childs = NULL;
    t->kstack = kstack;
    t->registers[0] = 0x0;
    t->registers[1] = 0xfffffffff000;
    t->registers[2] = arg;
    t->registers[10] = ( (uint64)(t->kstack + 0x10000) & 0xfffffffffffffff0);
    t->registers[11] = from_el1_to_el0;
    t->registers[12] = t->registers[10];
    memset(t->fd,0,sizeof(struct file*) * 65536);
    struct file* tmp;
    vfs_open("/dev/uart",64,&tmp);
    t->fd[0] = tmp;
    t->fd[1] = tmp;
    t->fd[2] = tmp;
    vfs_lookup("/",&(t->CurWorkDir));
    
    struct thread *temp = get_current();
    t->ptid = temp->tid;
    t->malloc_table[0] = NULL;
    struct thread_sibling *temp2 = temp->childs;
    struct thread_sibling *new_child = malloc(sizeof(struct thread_sibling));
    move_last_mem(t->tid);
    move_last_mem(t->tid);
    move_last_mem(t->tid);
    void *a;
    new_child->self = t;
    new_child->next = NULL;
    if(temp2 == NULL){
        temp->childs = new_child;
    }else{
        while(temp2->next != NULL){
            temp2 = temp2->next;
        }
        temp2->next = new_child;
    }
    PushToReadyList(t->tid);
    return t->tid;
}

int set_fork(void* sp){
    byte *t = get_current();
    tid_t tid = UserThread(return_to_child,NULL);
    pagetable_t* pagetable = threads[tid]->page_table;
    unsigned char* kstack = threads[tid]->kstack;

    byte *child = threads[tid];
    uint64 gap = (uint64)child - (uint64)t;
    for(int i=0;i<sizeof(struct thread);i++){
        child[i] = t[i];
    }

    struct thread *tmp = t;
    struct trapframe *tf = (uint64)kstack + (uint64)sp - (uint64)tmp->kstack;
    threads[tid]->tid = tid;
    threads[tid]->ptid = tmp->tid;
    threads[tid]->registers[10] = tf-1;
    threads[tid]->registers[11] = return_to_child;
    threads[tid]->registers[12] = tf;
    threads[tid]->next = NULL;
    threads[tid]->kstack = kstack;
    threads[tid]->page_table = pagetable;
    for(int i=0;i<0x10000;i++) kstack[i] = tmp->kstack[i];
    map_pages(threads[tid]->page_table,tmp->page_table);
    set_ttbr0_el1(tmp->page_table);
    tf->spsr_el1 = 0;
    tf->x[0] = 0;
    return tid;
}

void execute(char *file,char *const argv[]){
    void* code = NULL;
    uint64_t length = 0;
    copy_content(file, &code, &length);
    if(code == NULL || length == 0) {
        uart_printf("Error: \"%s\" is not an executable file\n",file);
        return;
    }
    int tid = UserThread(code,NULL);
    SetTaskCodePagetable(threads[tid]->page_table,code,length);
    InitUserTaskScheduler();
}

void exec(char *file,char *const argv[]){
    void* code = NULL;
    uint64_t length = 0;
    copy_content(file, &code, &length);
    if(code == NULL || length == 0){
        uart_printf("Error: \"%s\" is not an executable file\n",file);
        return;
    }
    struct thread *t = get_current();
    SetTaskCodePagetable(t->page_table,code,length);
    from_el1_to_el0(code, 0xfffffffff000);
}
