#pragma once
unsigned long long add_node(void (*callback_f)(),void* arguments,unsigned long long times, unsigned long long time_gap);
struct node* delete_first_node();
void print_node();
struct node{
    unsigned long long time_to_ring;
    void (*todo)();
    void *arguments;
    struct node* next;
};