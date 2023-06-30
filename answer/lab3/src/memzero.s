.globl memzero
memzero:
	str xzr, [x20], #8
	subs x21, x21, #8
	b.gt memzero
	ret
	