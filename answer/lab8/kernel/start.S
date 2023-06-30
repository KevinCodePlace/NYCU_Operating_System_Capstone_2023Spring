.section ".text.kernel"
.globl _start

#define PA(p) (p - 0xffff000000000000)

_start:
    ldr x9, =PA(_ekernel)
    mov sp, x9

    /* store dtb pointer */
    sub sp, sp, #0x10
    str x0, [sp]

    mov x9, sp
    msr sp_el1, x9

    /*          cpu id pass         */
    mrs    x20, mpidr_el1        
    and    x20, x20,#0xFF        // Check processor id
    cbz    x20, master        // Hang for all non-primary CPU

hang:
    b hang

master:
    ldr    x20, =PA(_sbss)
    ldr    x21, =PA(_ebss)
    sub    x21, x21, x20
    bl     memzero

    /* setup interrupt vector base */
    ldr x0, =el1_vector_base
    msr vbar_el1, x0
    bl     from_el2_to_el1   
      
    bl     setup_identity_mapping

    mov x1, #0
    movk x1, #0xffff, lsl 48
    
    mov x0, sp
    orr x0, x0, x1
    mov sp, x0

    mov x0, fp
    orr fp, x0, x1

    ldr x0, [sp]
    orr x0, x0, x1

    ldr x2, =_dtb_ptr
    str x0, [x2]
    
    ldr x9, =kernel_main
    br x9   

hang2:
    b hang2

.global _dtb_ptr
.section .data
_dtb_ptr: .dc.a 0x0