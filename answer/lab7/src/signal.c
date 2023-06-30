#include "thread.h"
#include "signal.h"
#include "scheduler.h"
#include "uint.h"
#include "queue.h"
#include "excep.h"
#include "mmu.h"
extern struct thread *threads[thread_numbers];

int signal(int SIGNAL, void (*handler)()){
    struct thread *t = get_current();
    t->sig_handler[SIGNAL] = handler;
}

int sentSignal(int pid, int SIGNAL){
    threads[pid]->signal |= 1<<SIGNAL;
}

void* sig_handler_kernel(struct thread *t){
    for(int i=0;i<32;i++){
        if((t->signal & 1<<i) && t->sig_handler[i] != NULL){
            t->signal &= !(1<<i);
            pagetable_t *pt = ((uint64_t)t->page_table & ~(uint64_t) 0b11) | 0xffff000000000000;
            void *sp = malloc(0x4000);
            pagetable_t *tmp_pt = allocate_page();
            t->page_table = ((uint64_t)tmp_pt & ~0xffff000000000000) | 0b11;
            SetTaskStackPagetable(t->page_table, sp);
            for(int j=0;j<0x1f0;j++) tmp_pt->entries[j] = pt->entries[j];
            set_ttbr0_el1(t->page_table);
            sig_handler_assembly(t->sig_handler[i], 0xfffffffff000, NULL);
            free(sp);
            t->page_table = pt;
            set_ttbr0_el1(t->page_table);
        }
    }
    return t;
}