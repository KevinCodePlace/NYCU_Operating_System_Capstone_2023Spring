.section ".text.kernel"

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

halt: 	
	wfe
	b	halt

setting: 
	adr x20, _sbss
	adr x21, _ebss
	sub x21, x21, x20

memzero: 
	str xzr,[x20],#8
	subs x21,x21,#8
	b.gt memzero

	mov sp,#0x400000
	bl main

.global _dtb_ptr
.section .data 
_dtb_ptr: .dc.a 0x0

