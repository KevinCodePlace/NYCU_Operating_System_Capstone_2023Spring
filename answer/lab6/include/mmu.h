#ifndef __MMU_H_
#define __MMU_H_

#include "exception_c.h"
typedef unsigned long pd_t;

#define PHYS_OFFSET 0xffff000000000000
#define PAR_PA_MASK 0xffffffff000L

#define PAGE_SIZE 0x1000
#define USTACK_VA 0xffffffffb000
#define STACK_SIZE 0x4000
#define UPROG_VA 0x0

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_PAGE 0b11
#define PD_ACCESS (1 << 10)
#define PD_USER_RW (0b01 << 6)
#define PD_USER_R (0b11 << 6)
#define PD_UXN (1L << 54)
#define PD_PXN (1L << 53)

// #define pa2va(p) (((unsigned long)(p + PHYS_OFFSET)))
// #define va2pa(p) (((unsigned long)(p - PHYS_OFFSET)))

static inline void *pa2va(unsigned long p)
{
    return (void *)(p + PHYS_OFFSET);
}

static inline unsigned long va2pa(void *p)
{
    return (unsigned long)p - PHYS_OFFSET;
}

#define IDENTITY_TT_L0 ((pd_t *)0x1000L)
#define IDENTITY_TT_L1 ((pd_t *)0x2000L)
#define IDENTITY_TT_L0_VA ((pd_t *)pa2va(0x1000L))
#define IDENTITY_TT_L1_VA ((pd_t *)pa2va(0x2000L))

void setup_identity_mapping();
void setup_kernel_space_mapping();
void map_page(pd_t *table, unsigned long va, unsigned long pa, unsigned long flags);
void *alloc_stack(pd_t *table, unsigned long size);
void *alloc_prog(pd_t *table, unsigned long size, const char *file_content);
void setup_peripheral_identity(pd_t *table);
unsigned long get_pte(unsigned long va);
unsigned long el0_va2pa(unsigned long va);
void free_page_table(pd_t *table);
void replace_page_table();

#endif
