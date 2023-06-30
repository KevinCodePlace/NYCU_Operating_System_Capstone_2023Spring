.section ".text"
.global _start
_start:
	svc	0x1337
1:
	nop
	b	1b
