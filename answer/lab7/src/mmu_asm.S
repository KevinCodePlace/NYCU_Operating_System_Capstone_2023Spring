#include "peripheral/mmu.h"
.global disable_mmu
.global set_tcr
.global set_mair
.global identity_paging
.global page_table_set
.global setup_mmu
.global set_ttbr0_el1

disable_mmu:
    ldr     x1, =0  //close MMU
    msr     sctlr_el1, x1
    ret

set_tcr:
    ldr     x0, = TCR_CONFIG_DEFAULT
    msr     tcr_el1, x0
    ret

set_mair:   
    ldr     x0, =( \
      (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
      (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
    )
    msr     mair_el1, x0
    ret

identity_paging:
    mov     x0, 0 // PGD's page frame at 0x0
    mov     x1, 0x1000 // PUD's page frame at 0x1000

    ldr     x2, = BOOT_PGD_ATTR
    orr     x2, x1, x2 // combine the physical address of next level page with attribute.
    str     x2, [x0]

    ldr     x2, = BOOT_PUD0_ATTR
    mov     x3, 0x2000
    orr     x3, x2, x3
    str     x3, [x1] // 1st 1GB mapped by the 1st entry of PUD
    ldr     x2, = BOOT_PUD1_ATTR
    mov     x3, 0x40000000
    orr     x3, x2, x3
    str     x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD

/*  mov     x1, 0x2000
    ldr     x2, = BOOT_PMD0_ATTR
    mov     x3, 0x10000     //PTE base
    mov     x4, xzr
    mov     x5, 0x200
    mov     x6, 8

1:  mul     x7, x4, x6
    lsl     x9, x4, 12
    add     x9, x3, x9
    orr     x9, x2, x9
    add     x8, x1, x7
    str     x9, [x8]
    add     x4, x4, #1
    cmp     x4, x5
    b.ls    1b*/

2:  mov     x1, 0x2000
    ldr     x2, = BOOT_PMD0_ATTR_RAM
    mov     x3, 0x1f7
    mov     x4, xzr

3:  lsl     x6, x4, 3
    add     x7, x1, x6
    lsl     x8, x4, 21
    orr     x8, x2, x8
    str     x8, [x7]
    add     x4, x4, #1
    cmp     x4, x3
    b.ls    3b

4:  cmp     x4, 0x1ff
    bge     5f
    ldr     x2, = BOOT_PMD0_ATTR_MMIO
    mov     x3, 0x200
    b       3b
    
5:  msr     ttbr0_el1, x0 // load PGD to the bottom translation-based register.
    msr     ttbr1_el1, x0

    mrs     x2, sctlr_el1
    orr     x2 , x2, 1
    msr     sctlr_el1, x2 // enable MMU, cache remains disabled
    
    
    ldr     x2, = boot_rest // indirect branch to the virtual address
    br      x2

setup_mmu:
    bl      set_tcr
    bl      set_mair
    bl      identity_paging
    ret

set_ttbr0_el1:
    dsb ish // ensure write has completed
    msr ttbr0_el1, x0 // switch translation based address.
    tlbi vmalle1is // invalidate all TLB entries
    dsb ish // ensure completion of TLB invalidatation
    isb // clear pipeline
    ret
.global get_ttbr0_el1
get_ttbr0_el1:
    mrs x0, ttbr0_el1
    ret