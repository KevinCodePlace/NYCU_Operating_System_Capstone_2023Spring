#ifndef __UTILS_S_H
#define __UTILS_S_H

#define read_sysreg(r) ({        \
    unsigned long __val;         \
    asm volatile("mrs %0, " #r   \
                 : "=r"(__val)); \
    __val;                       \
})

#define write_sysreg(r, __val) ({                \
    asm volatile("msr " #r ", %0" ::"r"(__val)); \
})

#define read_gpreg(r) ({         \
    unsigned long __val;         \
    asm volatile("mov %0, " #r   \
                 : "=r"(__val)); \
    __val;                       \
})

#define write_gpreg(r, __val) ({                 \
    asm volatile("mov " #r ", %0" ::"r"(__val)); \
})


// EC,bits[31:26]
#define ESR_ELx_EC(esr) ((esr & 0xFC000000) >> 26)
// ISS,bits[24:0]
#define ESR_ELx_ISS(esr) (esr & 0x03FFFFFF)

#define ESR_ELx_EC_SVC64 0b010101
#define ESR_ELx_EC_DABT_LOW 0b100100
#define ESR_ELx_EC_IABT_LOW 0b100000


void branchAddr(void *addr);
int get_el();
void switch_to(void *, void *);

#endif
