.section ".text.boot"

.global _start

_start:
	//load the address of the _dtb_ptr variable into the x1 register
	//ensure that x1 holds the correct address 
    ldr    x1, =_dtb_ptr
	//store the content of the x0 register into the memory location pointed to by x1.
    //x0 register is expected to contain the address of the Device Tree Blob (DTB) passed by the bootloader when the kernel starts.
	str    x0, [x1]  
	// read cpu id, stop slave cores
    mrs     x20, mpidr_el1
    and     x20, x20, #3
    cbz     x20, setting
    // cpu id > 0, stop

halt: 	wfe
	b	halt

setting: ldr x1, =_start
 	 mov sp, x1
	 ldr x1, =__bss_start
	 ldr w2, =__bss_size

clear_bss: cbz w2, kernel_main
	   str xzr,[x1],#8
	   sub w2, w2, #1
	   cbnz w2, clear_bss

kernel_main: bl main
     b  halt

.global _dtb_ptr
.section .data
//The .data section is used to store initialized global and static variables 
_dtb_ptr: .dc.a 0x0
// This directive is an assembler directive
// tells the assembler to reserve space for a pointer (an address) in the memory
// on a 64-bit system, it reserves 8 bytes.
