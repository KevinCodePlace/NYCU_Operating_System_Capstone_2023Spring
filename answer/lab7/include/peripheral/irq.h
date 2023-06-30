# pragma once

#include "uint.h"
#include "mmio.h"

#define IRQ_BASE (MMIO_BASE + 0x0000B000)


#define irq0_pending_0  ((volatile unsigned int*)(IRQ_BASE+0x200))
#define irq0_pending_1  ((volatile unsigned int*)(IRQ_BASE+0x204))
#define irq0_pending_2  ((volatile unsigned int*)(IRQ_BASE+0x208))
#define fiq_control     ((volatile unsigned int*)(IRQ_BASE+0x20C))
#define irq0_enable_1   ((volatile unsigned int*)(IRQ_BASE+0x210))
#define irq0_enable_2   ((volatile unsigned int*)(IRQ_BASE+0x214))
#define irq0_enable_0   ((volatile unsigned int*)(IRQ_BASE+0x218))
#define irq0_disable_1  ((volatile unsigned int*)(IRQ_BASE+0x21C))
#define irq0_disable_2  ((volatile unsigned int*)(IRQ_BASE+0x220))
#define irq0_disable_0  ((volatile unsigned int*)(IRQ_BASE+0x224))

