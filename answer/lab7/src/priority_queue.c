#include "allocator.h"
#include "priority_queue.h"
#include "uint.h"
#include "thread.h"

struct node *nodes = NULL;
int num_of_nodes=0;

uint64 add_node(void (*callback_f)(),void* arguments,uint64 times,uint64 time_gap){
    
    struct node *node = malloc(sizeof(struct node));
    node->time_to_ring = times;
    node->todo = callback_f;
    node->next = NULL;
    node->arguments = arguments;
    int a;
    if(nodes == NULL){
        nodes = node;
    }else if( time_gap >= times){
        time_gap = nodes->time_to_ring-time_gap;
        node->next = nodes;
        nodes = node;
        struct node *temp=nodes->next;
        while(temp != NULL){
            temp->time_to_ring -= time_gap;
            temp = temp->next;
        }
    }else{
        struct node* temp = nodes;
        time_gap = nodes->time_to_ring-time_gap;
        temp->time_to_ring -= time_gap;
        while(temp->next != NULL){
            temp->next->time_to_ring -= time_gap;
            if(temp->next->time_to_ring >= times){
                node->next = temp->next;
                temp->next = node;
                temp->next = temp->next->next;
                break;
            }else{
                temp = temp->next;
            }
        }
        if(temp->next == NULL){
            temp->next = node;
        }else{
            while(temp->next != NULL){
                temp->next->time_to_ring -= time_gap;
                temp = temp->next;
            }
        }
    }
    num_of_nodes++;
    return nodes->time_to_ring;
}

struct node* delete_first_node(){
    if(nodes == NULL) return NULL;
    uint64 times=nodes->time_to_ring;
    struct node *t = nodes;
    nodes = nodes->next;
    free(t);
    struct node *temp = nodes;
    while(temp != NULL){
        temp->time_to_ring -= times;
        temp = temp->next;
    }
    num_of_nodes--;
    return t;
}

void print_node(){
    struct node *temp = nodes;
    int i=0;
    while(temp != NULL){
        uart_printf("%d: %d\n",i,temp->time_to_ring);
        i++;
        temp = temp->next;
    }
}