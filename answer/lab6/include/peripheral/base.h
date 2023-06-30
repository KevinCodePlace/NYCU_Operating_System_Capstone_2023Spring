#ifndef _PERIPHERAL_BASE_H
#define _PERIPHERAL_BASE_H

#include "mmu.h"

#define MMIO_BASE           PHYS_OFFSET + 0x3F000000
#define MAILBOX_BASE        MMIO_BASE + 0xb880

#endif