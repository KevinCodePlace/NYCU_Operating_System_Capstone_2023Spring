#include "interrupt_queue.h"
#include "aux.h"
#include "allocator.h"

struct interrupt_event *task_controller = NULL;

void push_queue(void (*callback)()){
    struct interrupt_event *tmp = simple_malloc(sizeof(struct interrupt_event));
    tmp->callback = callback;
    tmp->next = NULL;
    if(task_controller == NULL){
        task_controller = tmp;
    }else{
        struct interrupt_event *tmp2 = task_controller;
        while(tmp2->next != NULL) tmp2 = tmp2->next;
        tmp2->next = tmp;
    }
}

int task_empty(){
    return task_controller == NULL;
}

void exe_first_task(){
    if(task_controller == NULL){ return;}
    struct interrupt_event* tmp = task_controller;
    task_controller = task_controller->next;
    tmp->callback();
    *AUX_MU_IER |= 1;
    exe_first_task();
}