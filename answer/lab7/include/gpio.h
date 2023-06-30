#include "peripheral/mmio.h"

#define GPIO_BASE       (MMIO_BASE + 0x200000)

#define GPFSEL0         ((volatile unsigned int*)(GPIO_BASE + 0x00))
#define GPFSEL1         ((volatile unsigned int*)(GPIO_BASE + 0x04))
#define GPFSEL2         ((volatile unsigned int*)(GPIO_BASE + 0x08))
#define GPFSEL3         ((volatile unsigned int*)(GPIO_BASE + 0x0C))
#define GPFSEL4         ((volatile unsigned int*)(GPIO_BASE + 0x10))
#define GPFSEL5         ((volatile unsigned int*)(GPIO_BASE + 0x14))
// 0x18 Reserved
#define GPSET0          ((volatile unsigned int*)(GPIO_BASE + 0x1C))
#define GPSET1          ((volatile unsigned int*)(GPIO_BASE + 0x20))
// 0x24 Reserved
#define GPCLR0          ((volatile unsigned int*)(GPIO_BASE + 0x28))
#define GPCLR1          ((volatile unsigned int*)(GPIO_BASE + 0x2C))
// 0x30 Reserved
#define GPLEV0          ((volatile unsigned int*)(GPIO_BASE + 0x34))
#define GPLEV1          ((volatile unsigned int*)(GPIO_BASE + 0x38))
// 0x3C Reserved
#define GPEDS0          ((volatile unsigned int*)(GPIO_BASE + 0x40))
#define GPEDS1          ((volatile unsigned int*)(GPIO_BASE + 0x44))
// 0x48 Reserved
#define GPREN0          ((volatile unsigned int*)(GPIO_BASE + 0x4C))
#define GPREN1          ((volatile unsigned int*)(GPIO_BASE + 0x50))
// 0x54 Reserved
#define GPFEN0          ((volatile unsigned int*)(GPIO_BASE + 0x58))
#define GPFEN1          ((volatile unsigned int*)(GPIO_BASE + 0x5C))
// 0x60 Reserved
#define GPHEN0          ((volatile unsigned int*)(GPIO_BASE + 0x64))
#define GPHEN1          ((volatile unsigned int*)(GPIO_BASE + 0x68))
// 0x6C Reserved
#define GPLEN0          ((volatile unsigned int*)(GPIO_BASE + 0x70))
#define GPLEN1          ((volatile unsigned int*)(GPIO_BASE + 0x74))
// 0x78 Reserved
#define GPAREN0         ((volatile unsigned int*)(GPIO_BASE + 0x7C))
#define GPAREN1         ((volatile unsigned int*)(GPIO_BASE + 0x80))
// 0x84 Reserved
#define GPAFEN0         ((volatile unsigned int*)(GPIO_BASE + 0x88))
#define GPAFEN1         ((volatile unsigned int*)(GPIO_BASE + 0x8C))
// 0x90 Reserved
#define GPPUD           ((volatile unsigned int*)(GPIO_BASE + 0x94))
#define GPPUDCLK0       ((volatile unsigned int*)(GPIO_BASE + 0x98))
#define GPPUDCLK1       ((volatile unsigned int*)(GPIO_BASE + 0x9C))
