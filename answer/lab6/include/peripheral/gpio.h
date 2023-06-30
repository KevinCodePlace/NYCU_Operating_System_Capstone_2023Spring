#ifndef	_PERIPHERAL_GPIO_H
#define	_PERIPHERAL_GPIO_H

#include "peripheral/base.h"

#define GPFSEL1         ((volatile unsigned int *)(MMIO_BASE+0x00200004))
#define GPSET0          ((volatile unsigned int *)(MMIO_BASE+0x0020001C))
#define GPCLR0          ((volatile unsigned int *)(MMIO_BASE+0x00200028))
#define GPPUD           ((volatile unsigned int *)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int *)(MMIO_BASE+0x00200098))

#endif