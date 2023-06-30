#ifndef __UTILS_S_H
#define __UTILS_S_H

#define read_sysreg(r) ({                       \
    unsigned long __val;                        \
    asm volatile("mrs %0, " #r : "=r" (__val)); \
    __val;                                      \
})

#define write_sysreg(r, __val) ({                  \
	asm volatile("msr " #r ", %0" :: "r" (__val)); \
})

#define read_gpreg(r) ({                       \
    unsigned long __val;                        \
    asm volatile("mov %0, " #r : "=r" (__val)); \
    __val;                                      \
})

#define write_gpreg(r, __val) ({                  \
	asm volatile("mov " #r ", %0" :: "r" (__val)); \
})

void branchAddr(void *addr);
int get_el();


#endif
