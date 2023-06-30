#pragma once
#define pte_t unsigned long long
#define size_t unsigned long
typedef struct{pte_t entries[512];} pagetable_t;
void disable_mmu();
void setup_mmu();
void set_ttbr0_el1(pagetable_t* pt);
pagetable_t* allocate_page();
void SetTaskStackPagetable(pagetable_t *pt, void* stack_addr);
void SetTaskCodePagetable(pagetable_t *pt, void* code_addr, unsigned long long size);
void SetPeripherialPagetable(pagetable_t *pt);
void* mmap_set(void* addr, size_t len, int prot, int flags);
unsigned char mmap_check(unsigned long long FAR);
void map_pages(unsigned long long des, unsigned long long src);
void cow_init();
unsigned long long get_ttbr0_el1();

