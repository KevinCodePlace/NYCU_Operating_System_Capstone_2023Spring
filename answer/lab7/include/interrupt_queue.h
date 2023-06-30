#pragma once
#include "uint.h"
struct interrupt_event{
    uint64 type;
    uint64 esr;
    uint64 elr;
    uint64 spsr;
    void (*callback)();
    struct interrupt_event *next;
};

void push_queue(void (*callback)());
int task_empty();
void exe_first_task();