#include "uint.h"
#include "mini_uart.h"
#include "allocator.h"
#include "math.h"
#include "dtb.h"
#include "string.h"
#define page_size 4096    //4KB
extern unsigned char __heap_start;

uint64_t base_addr;
uint64_t end_addr;

uint64_t frame_num;

struct FrameArray** frame_list;
struct FrameArray* frame_array;

struct mem_frag *pool;

struct mem_reserved_pool *MR_pool;


void* simple_malloc(unsigned int size) {
    uint64_t *temp=(uint64_t*)(&__heap_start-8);
    uint64_t addr = ((uint64_t) &__heap_start) + *temp;
    if(size%16 != 0){
        size /= 16;
        size *= 16;
        size += 16;
    }
    *temp+=size;
    return (void*) addr;
}

void init_allocator(){
    uint32_t *b,*e;
    b = find_property_value("/memory@0\0","reg\0");
    base_addr = letobe(*b);
    end_addr = letobe(*(b+1));
    base_addr |= 0xffff000000000000;
    end_addr |= 0xffff000000000000;
    frame_num = (end_addr - base_addr)/page_size;
    frame_array = simple_malloc(frame_num * sizeof(struct FrameArray));
    frame_array[0].val = log2(frame_num);
    frame_array[0].index = 0;
    frame_array[0].next =NULL;
    frame_array[0].allocatable = 1;
    for(int i=1;i<frame_num;i++){
        frame_array[i].val = -1;
        frame_array[i].index = i;
        frame_array[i].allocatable = 1;
        frame_array[i].next =NULL;
    }
    frame_list = simple_malloc((log2(frame_num)+1) * sizeof(struct FrameArray*));
    frame_list[0] = &frame_array[0];
    for(int i=1;i<(log2(frame_num)+1);i++){
        frame_list[i] = NULL;
    }
    pool = NULL;
    MR_pool = NULL;
}

void* page_alloc(unsigned int page_n){
    int logarithm = log2(page_n);
    struct mem_reserved_pool* f1;
    while(1){
        void* addr = getbestfit(logarithm);
        f1 = MR_pool;
        while(f1 != NULL){
            if(addr >= f1->start && addr <= f1->end){
                break;
            }
            f1 = f1->next;
        }
        if(f1 == NULL) return addr;
    }
}

void* getbestfit(int ind){
    int i;
    for(i=0;i+ind<=log2(frame_num);i++) 
        if(frame_list[(int)log2(frame_num)-ind-i] != NULL)
            break;
    while(i != 0){
        struct FrameArray* tmp=frame_list[(int)log2(frame_num)-ind-i];
        frame_list[(int)log2(frame_num)-ind-i] = (frame_list[(int)log2(frame_num)-ind-i])->next;
        unsigned int n= exp2(tmp->val-1);
        struct FrameArray* tmp2=&frame_array[tmp->index + n];
        tmp->next = tmp2;
        tmp->val -= 1;
        tmp2->next = NULL;
        tmp2->val = tmp->val;
        i--;
        frame_list[(int)log2(frame_num)-ind-i] = tmp;
    }

    struct FrameArray* tmp = frame_list[(int)log2(frame_num)-ind];
    frame_list[(int)log2(frame_num)-ind] = (frame_list[(int)log2(frame_num)-ind])->next;
    tmp->allocatable = 0;
    tmp->next = NULL;
    return (void*)(base_addr + page_size*tmp->index);
}

void show_status(){
    int i=0;
    uart_printf("0x%x: ",base_addr);
    while(i < frame_num){
        if(i != 0) uart_write('|');
        int length = exp2(frame_array[i].val);
        char a = frame_array[i].allocatable + '0';
        for(int j=0;j<length;j++) uart_write(a);
        i += length;
    }
    uart_printf(" 0x%x\n",end_addr);
}

void page_free(void* var){
    struct FrameArray* tmp = var;
    int index = ((int)var - base_addr) / page_size;
    if( frame_array[index].allocatable == 1 || ((int)var - base_addr) % page_size != 0) return;
    else{
        frame_array[index].allocatable = 1;
        merge(index);
    }
}

void merge(int index){
    int length = frame_array[index].val;
    int front,back;
    if(index == 0){
        front = 0;
        back = index + (int)exp2(length);
    }else{
        if(log2(index) > length){
            front = index;
            back = index + (int)exp2(length);
        }else{
            front = index - (int)exp2(length);
            back = index;
        }
    }
    if(frame_array[front].val == frame_array[back].val && frame_array[front].allocatable && frame_array[back].allocatable){
        struct FrameArray *tmp = frame_list[(int)log2(frame_num) - length];
        if(tmp->index == index + (int)exp2(length)){
            frame_list[(int)log2(frame_num) - length] = frame_list[(int)log2(frame_num) - length]->next;
        }
        else {
            int the_removed = index == front? back:front;
            while(tmp->next != NULL){
                if(tmp->next->index == the_removed){
                    tmp->next = tmp->next->next;
                    break;
                }
            }
        }
        frame_array[front].val += 1;
        frame_array[back].val = -1;
        merge(front);
    }else{
        struct FrameArray *tmp = frame_list[(int)log2(frame_num) - length];
        if(tmp == NULL || tmp->index > index) 
            frame_list[(int)log2(frame_num) - length] = &(frame_array[index]);
        else{
            while(tmp != NULL && tmp->next != NULL){
                if(tmp->next->index > index){
                    frame_array[index].next = tmp->next;
                    tmp->next = &(frame_array[index]);
                    break;
                }
            }
            if(tmp->next == NULL){
                tmp->next = &(frame_array[index]);
                frame_array[index].next =NULL;
            }
        }
    }
}

void insert_pool(struct mem_frag* f){
    struct mem_frag* tmp = pool;
    if(tmp == NULL){
        pool = f;
        return;
    }
    while(tmp->next != NULL) tmp = tmp->next;
    tmp->next = f;
}


void* malloc(size_t size){
    struct mem_frag* tmp = pool;
    if(size != exp2(log2(size))) size = exp2(log2(size) + 1);
    while(tmp != NULL){
        if(tmp->size == size && tmp->leave > 0){
            for(int i=0;i<tmp->num;i++){
                if(tmp->status[i] == 1){
                    tmp->status[i] = 0;
                    void* addr = tmp->start + tmp->size*i;
                    tmp->leave--;
                    record_mem(addr);
                    return addr;
                }
            }
            
        }
        tmp = tmp->next;
    }
    int req_page_num;
    req_page_num = size%page_size != 0? size/page_size+1 : size/page_size;
    req_page_num = exp2(log2(req_page_num)) != req_page_num? exp2(log2(req_page_num) + 1) : req_page_num;

    tmp = simple_malloc(sizeof(struct mem_frag));
    tmp->size = size;
    tmp->start = page_alloc(req_page_num);
    tmp->num = req_page_num * 4096 / size;
    tmp->end = tmp->start + req_page_num * 4096;
    tmp->status = simple_malloc(tmp->num);
    tmp->leave = tmp->num - 1;
    tmp->next = NULL;
    insert_pool(tmp);
    for(int i=0;i<tmp->num;i++){
        tmp->status[i] = 1;
    }
    tmp->status[0] = 0;
    void* addr = tmp->start;
    record_mem(addr);
    uint64_t res = (uint64_t)addr | 0xffff000000000000;
    return res;
}

void pool_status(){
    struct mem_frag* f = pool;
    uart_printf("Available pool:\n");
    for(int i=1;f!=NULL;i++){
        uart_printf("  %d: addr: 0x%x size: %d num: %d leave: %d\n",i,f->start,f->size,f->num,f->leave);
        f = f->next;
    }
    struct mem_reserved_pool* f1 = MR_pool;
    uart_printf("Reserved pool:\n");
    for(int i=1;f1!=NULL;i++){
        uart_printf("  %d: addr: 0x%x to 0x%x\n",i,f1->start,f1->end);
        f1 = f1->next;
    }
}

void free(void* addr){
    struct mem_frag* tmp = pool;
    while(tmp != NULL){
        if((uint64)addr < (uint64)tmp->end && (uint64)tmp->start <= (uint64)addr){
            if(tmp->status[(addr-tmp->start)/tmp->size] == 0){
                tmp->leave++;
                tmp->status[(addr-tmp->start)/tmp->size] = 1;
            }   
            return;
        }
        tmp = tmp->next;
    }
}

void clear_pool(){
    while(pool!=NULL){
        page_free(pool->start);
        pool = pool->next;
    }
}

void memory_reserve(uint64 start,uint64 end){
    if(end < start) return;
    struct mem_reserved_pool *tmp;
    tmp = MR_pool;
    if(tmp != NULL){
        if(start >= tmp->start && start <= tmp->end){
            if(end > tmp->end){tmp->end = end;
                MR_pool = MR_pool->next;
                return memory_reserve(tmp->start,tmp->end);
            }
            return;
        }else if(end >= tmp->start && end <= tmp->end){
            tmp->start = start;
            MR_pool = MR_pool->next;
            return memory_reserve(tmp->start,tmp->end);
        }
    }
    while(tmp != NULL && tmp->next != NULL){
        if(start >= tmp->next->start && start <= tmp->next->end){
            if(end > tmp->next->end){
                tmp->next->end = end;
                struct mem_reserved_pool *tmp2 = tmp->next;
                tmp->next = tmp2->next;
                return memory_reserve(tmp2->start,tmp2->end);
            }
            return;
        }else if(end >= tmp->next->start && end <= tmp->next->end){
            tmp->next->start = start;
            struct mem_reserved_pool *tmp2 = tmp->next;
            tmp->next = tmp2->next;
            return memory_reserve(tmp2->start,tmp2->end);
        }
        tmp = tmp->next;
    }
    tmp = simple_malloc(sizeof(struct mem_reserved_pool));
    tmp->start = start;
    tmp->end = end;
    tmp->next = MR_pool;
    MR_pool = tmp;
}