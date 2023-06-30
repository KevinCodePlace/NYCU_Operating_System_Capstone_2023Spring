.macro push_registers
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    str x9, [x0, 16 * 6]
.endm

.macro pop_registers
    ldp x19, x20, [x0, 16 * 0]
    ldp x21, x22, [x0, 16 * 1]
    ldp x23, x24, [x0, 16 * 2]
    ldp x25, x26, [x0, 16 * 3]
    ldp x27, x28, [x0, 16 * 4]
    ldp fp, lr, [x0, 16 * 5]
    ldr x9, [x0, 16 * 6]
    mov sp,  x9
.endm

.global switch_to
switch_to:
    push_registers

    mov x0, x1

    pop_registers
    msr tpidr_el1, x1
    mov x0, #0
    ret

.global get_current
get_current:
    mrs x0, tpidr_el1
    ret

.global store_and_jump
store_and_jump:
    push_registers

    ldr x0, [x1, 8 * 2]
    ldr x2, [x1, 8 * 3]
    ldr x3, [x1, 8 * 4]

    mov x29, sp

    ldr x9, [x1, 8 * 1]
    mov sp, x9

    stp   x1, x29, [sp, #-16]!
    mov   fp, sp

    ldr x9, [x1, 8 * 0]
    msr tpidr_el1, x1
    mov x1, sp
    blr x9

    ldp  x0, x29, [sp, 16 * 0]
    mov  sp, x29

    mov  w1, 3
    str  w1, [x0, 140]

    bl   exit

.global set_current
set_current:
    ldr x9, [x0, 8 * 1]
    mov sp, x9

    mov fp, sp

    msr tpidr_el1, x0
    ldr x9, [x0, 8 * 0]
    blr  x9

.global set_proc
set_proc:
    msr tpidr_el1, x0
    ret

.global task_schedule
task_schedule:
    msr daifclr, 0x0
    push_registers
    
    bl  UserScheduler

.global SwitchTo
SwitchTo:
    pop_registers
    msr tpidr_el1, x0
    b  check_signal
    ret

.global sig_handler_assembly
sig_handler_assembly:
    mov x19, x0
    mov x0, x2
    sub sp, sp, #16
    str lr, [sp, 0 * 16]
    msr daifclr, 0xf
    bl  from_el1_to_el0_forsig
    mov x0, x19
    blr x19                            //el0 start
    mov x8, #20
    svc #0

.global ret_to_sig_han
ret_to_sig_han:
    msr daifclr, 0x0
    mov sp, x0
    ldr lr, [sp, 0 * 16]
    add sp, sp, #16
    ret

.global check_signal
check_signal:
    mov x1, sp
    sub x1, x1, 16 * 7
    mov x2, x0
    mov x0, x1
    push_registers
    mov sp, x1
    mov x0, x2

    bl  sig_handler_kernel

    mov x0, sp
    pop_registers
    msr daifclr, 0xf
    ret

from_el1_to_el0_forsig:
    msr     elr_el1, lr

    msr     sp_el0,  x1

    ldr     x1, =0
    msr     spsr_el1, x1


    eret

.global get_el
get_el:
    mrs x0, CurrentEL
    lsr x0, x0, #2
    and x0, x0, #3
    ret

.global call_exit
call_exit:
    mov x8, #5
    svc #0