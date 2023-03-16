.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, setting
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
