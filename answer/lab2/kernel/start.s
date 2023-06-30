.section ".text.kernel"
.globl _start

_start:
    ldr    x1, =_dtb_ptr
    str    x0, [x1]   
    mrs    x20, mpidr_el1        
    and    x20, x20,#0xFF        // Check processor id
    cbz    x20, master        // Hang for all non-primary CPU

hang:
    b hang

master:
    adr    x20, _sbss
    adr    x21, _ebss
    sub    x21, x21, x20
    bl     memzero

    mov    sp, #0x400000  // 4MB
    bl    kernel_main
    

.global _dtb_ptr
.section .data
_dtb_ptr: .dc.a 0x0
