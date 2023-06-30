.section ".text.relocate"

.global _relocate

_relocate:
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
    // save dtb location
    ldr     x1, =__dtb_addr
    str     x0, [x1]
    
    // set stack pointer
    ldr     x1, =__relocation
    mov     sp, x1


    // clear bss
    ldr     x1, =__bss_start
    ldr     x2, =__bss_size
3:  cbz     x2, 4f
    str     xzr, [x1], #8
    sub     x2, x2, #1
    cbnz    x2, 3b

4:  bl      relocate


.section ".text.boot"

.global _start

_start:
    // jump to main function in C
    bl      main
    // halt this core if return
1:
    wfe
    b       1b