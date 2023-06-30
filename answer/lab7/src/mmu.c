#include "mmu.h"
#include "peripheral/mmu.h"
#include "uint.h"
#include "allocator.h"
#include "math.h"
#include "thread.h"
#include "scheduler.h"

struct pte_manage
{
    byte prot,num;
}*pm;

void cow_init(){
    pm = malloc(sizeof(struct pte_manage) * (0x40000000/0x1000));
    delete_last_mem(pm);
    for(int i=0;i<(0x40000000/0x1000);i++){
        pm[i].prot = 0;
        pm[i].num = 0;
    }
}

uint64_t vm_decode(uint64_t va, pagetable_t *p){
    pagetable_t *pt = ((uint64_t) p & 0xfffff000) | 0xffff000000000000;
    uint64_t blocksize = 0x8000000000;
    for(int i=0;i<4;i++){
        pt = ((uint64_t)pt->entries[va/blocksize] & 0xfffff000) | 0xffff000000000000;
        va %= blocksize;
        blocksize >>= 9;
        if(pt == 0xffff000000000000){
            return NULL;
        }
    }
    uint64_t re = ((uint64_t)pt&0xfffffffff000) + va;
    return re;
}

pagetable_t* allocate_page(){
    pagetable_t* pt = malloc(sizeof(pagetable_t));
    for(int i=0;i<512;i++) pt->entries[i] = NULL;
    return pt;
}

pte_t walk(pagetable_t *pt, uint64_t va, uint64_t end, uint64_t pa, uint64_t blocksize, byte prot){
    if(blocksize < 1<<12){
        uint32_t idx = (pa & 0xfffff000) >> 12;
        uint64_t label = PTE_ATTR_BASE;
        pm[idx].num=1;
        pm[idx].prot = prot;
        if(prot == 0) return (pa & 0xfffff000);
        if(prot & PROT_WRITE == 0) label |= RO_BIT | PD_ACCESS;
        else label |= PD_ACCESS;
        if(prot & PROT_EXEC == 0) label |= USR_EXE_NEVER_BIT;
        return (pa & 0xfffff000) | label;
    }else{
        uint64_t start = va / blocksize;
        uint64_t num = upper_bound(end,blocksize) - va/blocksize;
        if(pt == NULL){
            pt = allocate_page();
            move_last_mem(0);
        }else{
            pt = ((uint64_t)pt & ~(uint64_t)0x3) | 0xffff000000000000;
        }
        for(int i=0;i<num;i++){
            uint64_t va2;
            if(i==0) va2 = va;
            else va2 = (va/blocksize + i) * blocksize;
            uint64_t gap = (va2/blocksize) * blocksize;
            pt->entries[start + i] = walk(pt->entries[start + i], va2 - gap,min(end,(va2/blocksize + 1) * blocksize) - gap,pa + va2 - va,blocksize >> (uint64_t)9,prot);
        }
        return ((uint64_t)pt & (uint64_t)0xffffffff) | 0b11;
    }
}

void mappages(pagetable_t* pagetable, uint64_t va, uint64_t size, uint64_t pa, byte prot){
    uint64_t end_addr = va + size;
    walk(pagetable, va, end_addr, pa, (uint64_t)1 << 39, prot);
}

void SetTaskStackPagetable(pagetable_t *pt, void* stack_addr){
    int stack_size = 0x4000;
    mappages(pt,0xffffffffb000,stack_size, (uint64_t)stack_addr & 0xfffffffc,PROT_READ | PROT_WRITE);
}

void SetTaskCodePagetable(pagetable_t *pt, void* code_addr, uint64_t size){
    mappages(pt,0x0, size, (uint64_t)code_addr & 0xfffffffc, PROT_READ | PROT_EXEC);
}

void SetPeripherialPagetable(pagetable_t *pt){
    mappages(pt,0x3c000000, 0x4000000, 0x3c000000, PROT_READ | PROT_WRITE);
}

void* mmap_set(void* addr, size_t len, int prot, int flags){
    uint64_t align_addr = (uint64_t)addr & ~(uint64_t)0xfff;
    thread_t *t = get_current();
    while(vm_decode(align_addr,t->page_table) != NULL){
        align_addr += 0x1000;
        if(align_addr >= 0xffffffffb000) return NULL;
    }
    for(int i=0;i<len>>12;i++) mappages(t->page_table,align_addr + i * 0x1000,0x1000,(uint64_t)prot<<12,0);

    return align_addr;
}

bool mmap_check(uint64_t FAR){
    thread_t *t = get_current();
    if(FAR >= 0xFFFFFFFFB000 && FAR < 0xFFFFFFFFF000){
        uint64_t pa = vm_decode(FAR & ~(uint64_t)0xfff,t->page_table);
        if(pa == NULL){
            mappages(t->page_table,FAR & ~(uint64_t)0xfff,0x1000, (uint64_t)malloc(0x1000),PROT_READ | PROT_WRITE);
        }else{
            byte *stack = (pa & ~(uint64_t)0xfff) | 0xffff000000000000;
            byte *new_stack = malloc(0x1000);
            for (int i=0;i<0x1000;i++) new_stack[i] = stack[i];
            mappages(t->page_table,FAR & ~(uint64_t)0xfff,0x1000, new_stack,PROT_READ | PROT_WRITE);
        }
        set_ttbr0_el1(t->page_table);
        return true;
    }
    if(FAR >= 0x3c000000 && FAR < 0x40000000){
        SetPeripherialPagetable(t->page_table);
        set_ttbr0_el1(t->page_table);
        return true;
    }
    uint64_t align_addr = (uint64_t)FAR & ~(uint64_t)0xfff;
    uint64_t addr = vm_decode(align_addr,t->page_table)>>12;
    if(addr != 0){
        if(addr < 0x10000){
            uint64_t *pa = malloc(0x1000);
            for(int i=0;i<200;i++) pa[i] = 0;
            mappages(t->page_table,align_addr,0x1000,pa,addr);
        }else{
            if((pm[addr].prot&PROT_WRITE) == 0) return false;
            uint64_t *space = (addr<<12) | 0xffff000000000000;
            uint64_t *new_space = malloc(0x1000);
            for(int i=0;i<0x200;i++) new_space[i] = space[i];
            mappages(t->page_table,align_addr,0x1000,new_space,pm[addr].prot);
        }
        set_ttbr0_el1(t->page_table);
        return true;
    }
    return false;
}


void map_pages(uint64_t des, uint64_t src){
    pagetable_t* pt_des = (des&~(uint64_t)0xfff) | 0xffff000000000000;
    pagetable_t* pt_src = (src&~(uint64_t)0xfff) | 0xffff000000000000;
    for(int i=0;i<512;i++){
        if(pt_src->entries[i] == NULL) continue;
        pagetable_t* pt_des_l2 = allocate_page();
        pt_des->entries[i] = ((uint64_t)pt_des_l2 & ~0xffff000000000000) | 0b11;
        move_last_mem(0);
        pagetable_t* pt_src_l2 = ((uint64_t)pt_src->entries[i] & ~(uint64_t)0xfff) | 0xffff000000000000;
        for(int j=0;j<512;j++){
            if(pt_src_l2->entries[j] == NULL) continue;
            pagetable_t* pt_des_l1 = allocate_page();
            pt_des_l2->entries[i] = ((uint64_t)pt_des_l1 & ~0xffff000000000000) | 0b11;
            move_last_mem(0);
            pagetable_t* pt_src_l1 = ((uint64_t)pt_src_l2->entries[j] & ~(uint64_t)0xfff) | 0xffff000000000000;
            for(int k=0;k<512;k++){
                if(pt_src_l1->entries[k] == NULL) continue;
                pagetable_t* pt_des_l0 = allocate_page();
                pt_des_l1->entries[i] = ((uint64_t)pt_des_l0 & ~0xffff000000000000) | 0b11;
                move_last_mem(0);
                pagetable_t* pt_src_l0 = ((uint64_t)pt_src_l1->entries[k] & ~(uint64_t)0xfff) | 0xffff000000000000;
                for(int l=0;l<512;l++){
                    if(pt_src_l0->entries[l] == NULL) continue;
                    pm[((uint32_t)(pt_src_l0->entries[l]))>>12].num++;
                    pt_src_l0->entries[l] |= RO_BIT;
                    pt_des_l0->entries[l] = pt_src_l0->entries[l];
                }
            }
        }
    }
}