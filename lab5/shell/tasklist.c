#include "header/tasklist.h"
#include "header/allocator.h"
#include "header/uart.h"

task_t *task_head = NULL;

void enqueue_task(task_t *new_task) {
    // Disable interrupts to protect the critical section
    asm volatile("msr DAIFSet, 0xf");
    // Special case: the list is empty or the new task has higher priority
    if (!task_head || new_task->priority < task_head->priority) {
		new_task->next = task_head;
        new_task->prev = NULL;
        if (task_head) {
            task_head->prev = new_task;
        }
        task_head = new_task;
    } else {
        // Find the correct position in the list
        task_t *current = task_head;
        while (current->next && current->next->priority <= new_task->priority) {
            current = current->next;
        }

        // Insert the new task
        new_task->next = current->next;
        new_task->prev = current;
        if (current->next) {
            current->next->prev = new_task;
        }
        current->next = new_task;
    }

    // Enable interrupts
    asm volatile("msr DAIFClr, 0xf");
}

void create_task(task_callback callback, uint64_t priority) {

	task_t* task = simple_malloc(sizeof(task_t));
	if(!task) {
		return;
	}

	task->callback = callback;
	task->priority = priority;
		
	enqueue_task(task);
}

void execute_tasks() {
	

    while (task_head) {        
        task_head->callback();
        task_head = task_head->next;
        if (task_head) {
            task_head->prev = NULL;
        }
		asm volatile("msr DAIFSet, 0xf"); // Disable interrupts
        //simple_free(task);
    }

    asm volatile("msr DAIFClr, 0xf"); // Enable interrupts
}

