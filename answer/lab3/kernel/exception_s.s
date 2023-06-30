// save general registers to stack
.macro save_all
    sub sp, sp, 32 * 8
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
.endm

// load general registers from stack
.macro load_all
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
    add sp, sp, 32 * 8
.endm

.macro exception_entry label
    .align 7
    b     \label                    // branch to a handler function.
    
.endm	


.global el1_vector_base

.align 11
el1_vector_base:
  exception_entry exception_handler
  exception_entry exception_handler
  exception_entry exception_handler
  exception_entry exception_handler

  
  exception_entry _el1_curr_el_spx_sync
  exception_entry _el1_curr_el_spx_irq
  exception_entry exception_handler
  exception_entry exception_handler

  
  exception_entry _el1_lower_el_aarch64_sync
  exception_entry _el1_lower_el_aarch64_irq
  exception_entry exception_handler
  exception_entry exception_handler

  
  exception_entry exception_handler
  exception_entry exception_handler
  exception_entry exception_handler
  exception_entry exception_handler


_el1_lower_el_aarch64_sync:
  save_all
  bl lower_sync_handler
  load_all
  eret

_el1_lower_el_aarch64_irq:
  save_all
  bl lower_irq_handler
  load_all
  eret

_el1_curr_el_spx_sync:
  save_all
  bl curr_sync_handler
  load_all
  eret
  
_el1_curr_el_spx_irq:
  save_all
  bl curr_irq_handler_decouple
  load_all
  eret

exception_handler:
  save_all
  bl default_handler
  load_all
  eret 
