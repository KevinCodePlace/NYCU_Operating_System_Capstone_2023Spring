.section ".text.relo"
.globl _start

# need to relocate the bootloader from 0x80000 to 0x60000
_start:
    adr x10, .          //x10=0x80000
    ldr x11, =_blsize 
    add x11, x11, x10 
    ldr x12, =_stext    // x12=0x60000

moving_relo:
    cmp x10, x11        //without bootloader
    b.eq end_relo
    ldr x13, [x10]
    str x13, [x12]      //move 0x80000 data to 0x60000
    add x12, x12, #8
    add x10, x10, #8
    b moving_relo
end_relo:
    ldr x14, =_bl_entry    //jump to boot part 
    br x14


.section ".text.boot"
.globl _start_bl
    ldr x20, =_dtb
    str x0, [x20]
    mrs    x20, mpidr_el1        
    and    x20, x20,#0xFF // Check processor id
    cbz    x20, master   // Hang for all non-primary CPU

hang:
    b hang

master:
    adr    x20, _sbss
    adr    x21, _ebss
    sub    x21, x21, x20
    bl     memzero

    mov    sp, #0x400000    // 4MB
    bl    bootloader_main
    
.global _dtb
.section .data
_dtb: .dc.a 0x0
