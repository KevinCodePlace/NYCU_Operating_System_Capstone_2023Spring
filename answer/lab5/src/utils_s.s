.global branchAddr
branchAddr:
   br x0


.globl get_el
get_el:
   mrs x0, CurrentEl
   lsr x0, x0, #2
   ret

.global switch_to
switch_to:
    mov x9, sp
    stp x19, x20, [x0, #0x0]
    stp x21, x22, [x0, #0x10]
    stp x23, x24, [x0, #0x20]
    stp x25, x26, [x0, #0x30]
    stp x27, x28, [x0, #0x40]
    stp fp, x9,  [x0, #0x50]
    str lr, [x0, #0x60]

    ldp x19, x20, [x1, #0x0]
    ldp x21, x22, [x1, #0x10]
    ldp x23, x24, [x1, #0x20]
    ldp x25, x26, [x1, #0x30]
    ldp x27, x28, [x1, #0x40]
    ldp fp, x9,  [x1, #0x50]
    ldr lr, [x1, #0x60]
    mov sp,  x9
    msr tpidr_el1, x1
    ret
