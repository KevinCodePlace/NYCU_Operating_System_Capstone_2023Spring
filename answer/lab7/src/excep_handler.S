// save general registers to stack
.macro save_all
    sub sp, sp, #0x110
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    str x30, [sp, 16 * 15]

    mrs x0, sp_el0
    str x0, [sp, 16 * 15 + 8]
    mrs x0, spsr_el1
    mrs x1, elr_el1
    stp x0, x1, [sp ,16 * 16]
.endm

// load general registers from stack
.macro load_all

    ldp x0, x1, [sp ,16 * 16]
    msr spsr_el1, x0
    msr elr_el1, x1

    ldr x0, [sp, 16 * 15 + 8]
    msr sp_el0, x0
    
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    ldr x30, [sp, 16 * 15]    

    add sp, sp, #0x110
.endm

.macro exception_handler type
    save_all
    mov x0, #\type
    mrs x1, esr_el1
    mrs x2, elr_el1
    mrs x3, spsr_el1
    mov x4, sp
    bl     exception_entry
    load_all
    eret
.endm

.align 11 // vector table should be aligned to 0x800
.global exception_table
exception_table:
  b sync_1// branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b irq_1
  .align 7
  b fir_1
  .align 7
  b serror_1
  .align 7

  b sync_2// branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b irq_2
  .align 7
  b fir_2
  .align 7
  b serror_2
  .align 7

  b sync_3// branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b irq_3
  .align 7
  b fir_3
  .align 7
  b serror_3
  .align 7

  b sync_4// branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b irq_4
  .align 7
  b fir_4
  .align 7
  b serror_4
  .align 7

sync_1:
    exception_handler 0
irq_1:
    exception_handler 1
fir_1:
    exception_handler 2
serror_1:
    exception_handler 3

sync_2:
    exception_handler 4
irq_2:
    exception_handler 5
fir_2:
    exception_handler 6
serror_2:
    exception_handler 7

sync_3:
    exception_handler 8
irq_3:
    exception_handler 9
fir_3:
    exception_handler 10
serror_3:
    exception_handler 11

sync_4:
    exception_handler 12
irq_4:
    exception_handler 13
fir_4:
    exception_handler 14
serror_4:
    exception_handler 15

.global err_han
err_han: b err_han

.global return_to_child
return_to_child:
    load_all   
    eret
