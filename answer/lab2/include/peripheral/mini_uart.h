#ifndef _PERIPHERAL_MINI_UART_H
#define _PERIPHERAL_MINI_UART_H

#include "peripheral/base.h"

#define AUX_ENABLE      ((volatile unsigned int *)(MMIO_BASE + 0x00215004))
#define AUX_MU_IO_REG   ((volatile unsigned int *)(MMIO_BASE + 0x00215040))
#define AUX_MU_IER_REG  ((volatile unsigned int *)(MMIO_BASE + 0x00215044))
#define AUX_MU_IIR_REG  ((volatile unsigned int *)(MMIO_BASE + 0x00215048))
#define AUX_MU_LCR_REG  ((volatile unsigned int *)(MMIO_BASE + 0x0021504C))
#define AUX_MU_MCR_REG  ((volatile unsigned int *)(MMIO_BASE + 0x00215050))
#define AUX_MU_LSR_REG  ((volatile unsigned int *)(MMIO_BASE + 0x00215054))
#define AUX_MU_MSR_REG  ((volatile unsigned int *)(MMIO_BASE + 0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int *)(MMIO_BASE + 0x0021505C))
#define AUX_MU_CNTL_REG ((volatile unsigned int *)(MMIO_BASE + 0x00215060))
#define AUX_MU_STAT_REG ((volatile unsigned int *)(MMIO_BASE + 0x00215064))
#define AUX_MU_BAUD_REG ((volatile unsigned int *)(MMIO_BASE + 0x00215068))

#endif