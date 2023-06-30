#pragma once
#include "mmio.h"
#define CLOCKHZ 1000000

#define TIMER_BASE                   (MMIO_BASE + 0x3000)

#define SYSTEM_TIMER_CONTROL         ((volatile unsigned int*)(TIMER_BASE + 0x00))
#define SYSTEM_TIMER_COUNTER_L_32    ((volatile unsigned int*)(TIMER_BASE + 0x04))
#define SYSTEM_TIMER_COUNTER_H_32    ((volatile unsigned int*)(TIMER_BASE + 0x08))
#define SYSTEM_TIMER_COMPARE0        ((volatile unsigned int*)(TIMER_BASE + 0x0C))
#define SYSTEM_TIMER_COMPARE1        ((volatile unsigned int*)(TIMER_BASE + 0x10))
#define SYSTEM_TIMER_COMPARE2        ((volatile unsigned int*)(TIMER_BASE + 0x14))
#define SYSTEM_TIMER_COMPARE3        ((volatile unsigned int*)(TIMER_BASE + 0x18))