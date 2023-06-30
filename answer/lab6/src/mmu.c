#include "mmu.h"
#include "utils_s.h"
#include "utils_c.h"
#include "mini_uart.h"
#include "mm.h"
#include "current.h"

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) | (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define MMU_ENABLE 1
#define PAGE_SIZE 0x1000

#define PERIPHERAL_START 0x3c000000
#define PERIPHERAL_END 0x3f000000

void setup_identity_mapping()
{
    write_sysreg(tcr_el1, TCR_CONFIG_DEFAULT);
    size_t mair_attr_01 =
        (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) |
        (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8));
    write_sysreg(mair_el1, mair_attr_01);

    memset(IDENTITY_TT_L0, 0, 0x1000);
    memset(IDENTITY_TT_L1, 0, 0x1000);

    IDENTITY_TT_L0[0] = (pd_t)IDENTITY_TT_L1 | BOOT_PGD_ATTR;
    IDENTITY_TT_L1[0] = 0x00000000 | BOOT_PUD_ATTR;
    IDENTITY_TT_L1[1] = 0x40000000 | BOOT_PUD_ATTR;

    write_sysreg(ttbr0_el1, IDENTITY_TT_L0);
    write_sysreg(ttbr1_el1, IDENTITY_TT_L0); // also load PGD to the upper translation based register.
    unsigned long sctlr = read_sysreg(sctlr_el1);
    write_sysreg(sctlr_el1, sctlr | MMU_ENABLE);
}

void setup_kernel_space_mapping()
{
    /*  three-level 2MB block mapping    */

    /*  0x00000000 ~ 0x3F000000 for normal mem  */
    pd_t *p0 = kcalloc(PAGE_SIZE);
    for (int i = 0; i < 504; i++)
    {
        p0[i] = (i << 21) | PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK;
    }
    /*  0x3F000000 ~ 0x40000000 for device mem  */
    for (int i = 504; i < 512; i++)
    {
        p0[i] = (i << 21) | PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK;
    }

    /*  0x40000000 ~ 0x80000000 for device mem  */
    pd_t *p1 = kcalloc(PAGE_SIZE);
    for (int i = 0; i < 512; i++)
    {
        p1[i] = 0x40000000 | (i << 21) | PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK;
    }

    asm volatile("dsb ish\n\t");

    IDENTITY_TT_L1_VA[0] = (pd_t)va2pa(p0) | PD_TABLE;
    IDENTITY_TT_L1_VA[1] = (pd_t)va2pa(p1) | PD_TABLE;
}

void map_page(pd_t *table, unsigned long va, unsigned long pa, unsigned long flags)
{
    for (int level = 0; level < 4; level++)
    {
        unsigned idx = (va >> (39 - 9 * level)) & 0b111111111;
        if (!table[idx])
        {
            if (level == 3)
            {
                table[idx] = pa;
                table[idx] |= PD_PXN | PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE | flags;
                return;
            }
            else
            {                
                table[idx] = (pd_t)va2pa(kcalloc(PAGE_SIZE));
                table[idx] |= PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_TABLE;
            }
        }

        if ((table[idx] & 0b11) == PD_TABLE)
        {
            table = (pd_t *)pa2va(table[idx] & 0xfffffffff000L);
        }
    }
}

void *alloc_stack(pd_t *table, unsigned long size)
{
    void *stack = kcalloc(size);
    unsigned long pages = align_up(size, PAGE_SIZE) / PAGE_SIZE;
    for (int i = 0; i < pages; i++)
    {
        map_page(table, USTACK_VA + i * PAGE_SIZE, va2pa(stack + i * PAGE_SIZE), PD_USER_RW | PD_UXN);
    }

    return stack;
}

void *alloc_prog(pd_t *table, unsigned long size, const char *file_content)
{
    void *prog = kcalloc(size);
    unsigned long pages = align_up(size, PAGE_SIZE) / PAGE_SIZE;
    for (int i = 0; i < pages; i++)
    {
        map_page(table, UPROG_VA + i * PAGE_SIZE, va2pa(prog + i * PAGE_SIZE), PD_USER_RW);
    }
    memcpy(prog, file_content, size);

    return prog;
}

void setup_peripheral_identity(pd_t *table)
{
    unsigned long pages = align_up(PERIPHERAL_END - PERIPHERAL_START, PAGE_SIZE) / PAGE_SIZE;
    for (int i = 0; i < pages; i++)
    {
        map_page(table, PERIPHERAL_START + i * PAGE_SIZE, PERIPHERAL_START + i * PAGE_SIZE, PD_USER_RW);
    }
}

unsigned long get_pte(unsigned long va)
{
    unsigned long flags = disable_irq();

    asm("at s1e0r, %0\n\t" ::"r"(va));
    unsigned long pte = read_sysreg(par_el1);

    irq_restore(flags);
    return pte;
}

unsigned long el0_va2pa(unsigned long va)
{
    unsigned long entry = get_pte(va);
    if (entry & 1)
    {
        uart_printf("Failed map virtual addr at 0x%x\n", va);
    }
    unsigned long offset = va & 0xfff;
    return (unsigned long)(pa2va((entry & 0xfffffffff000L) | offset));
}
void free_page_table(pd_t *table)
{
    //  free the page table created by translating from  map_page()
    /*  TODO    */
}

void replace_page_table()
{
    asm(
        "dsb ish\n"           // ensure write has completed
        "msr ttbr0_el1, %0\n" // switch translation based address.
        "tlbi vmalle1is\n"    // invalidate all TLB entries
        "dsb ish\n"           // ensure completion of TLB invalidatation
        "isb\n"               // clear pipeline
        ::"r"(current->ttbr0));
}