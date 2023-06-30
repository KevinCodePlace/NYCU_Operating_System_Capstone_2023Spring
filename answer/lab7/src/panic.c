#include "mini_uart.h"
const char *entry_error_messages[] = {
    "SYNC_INVALID_EL1t",
    "IRQ_INVALID_EL1t",
    "FIQ_INVALID_EL1t",
    "ERROR_INVALID_EL1T",

    "SYNC_INVALID_EL1h",
    "IRQ_INVALID_EL1h",
    "FIQ_INVALID_EL1h",
    "ERROR_INVALID_EL1h",

    "SYNC_INVALID_EL0_64",
    "IRQ_INVALID_EL0_64",
    "FIQ_INVALID_EL0_64",
    "ERROR_INVALID_EL0_64",

    "SYNC_INVALID_EL0_32",
    "IRQ_INVALID_EL0_32",
    "FIQ_INVALID_EL0_32",
    "ERROR_INVALID_EL0_32",
};

void not_implemented() {
    uart_printf("kenel panic because of not implemented function...\n");
    while (1);
}

void show_exception_status(int type, unsigned long esr, unsigned long address) {
    uart_printf("%s, ESR: 0x%x, address: 0x%x\n", entry_error_messages[type], esr, address);
    uart_printf("Exception class (EC) 0x%x\n", (esr >> 26) & 0b111111);
    uart_printf("Instruction specific syndrome (ISS) 0x%x\n", esr & 0xFFFFFF);
}