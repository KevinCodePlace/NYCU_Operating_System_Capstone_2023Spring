
.section ".text.boot"

.global _start
.global from_el1_to_el0
.global user_process
.global boot_rest


_start:
    // get cpu id
    mrs     x1, MPIDR_EL1
    and     x1, x1, #3
    cbz     x1, 2f
    // if cpu_id > 0, stop
1:
    wfe
    b       1b
    // if cpu_id == 0
2:
    mov     x1, 0x02080000
    str     x0, [x1]
    bl      disable_mmu
    
    // set stack pointer
    ldr     x1, =_start
    mov     sp, x1

    bl      from_el2_to_el1
    bl      setup_mmu

boot_rest:
    // clear bss
    ldr     x1, =__bss_start
    ldr     x2, =__bss_size
3:  cbz     x2, 4f
    str     xzr, [x1], #8
    sub     x2, x2, #1
    cbnz    x2, 3b

    

4:  bl      main
    // halt this core if return
    b       1b


from_el2_to_el1:
    ldr     x1, =_start
    msr     sp_el1, x1

    mov     x1, (0b11 << 20) // make el0, el1 can use Floating point and Advanced SIMD
    msr     CPACR_EL1, x1

    mov     x0, (1 << 31) // EL1 uses aarch64
    msr     hcr_el2, x0

    mov     x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr     spsr_el2, x0

    msr     elr_el2, lr

    
    adr     x9, exception_table
    msr     vbar_el1, x9
    
    eret

from_el1_to_el0:
    msr     elr_el1, x19

    msr     sp_el0,  x20

    ldr     x1, =0x3c0
    msr     spsr_el1, x1


    eret

