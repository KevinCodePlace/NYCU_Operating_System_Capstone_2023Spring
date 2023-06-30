#include "mini_uart.h"
#include "shell.h"
#include "mailbox.h"
#include "loadimg.h"
#define max_length 128


int main() {
    shell_init();
    uart_printf("Initial completed\n");
    get_board_revision();
    get_arm_memory();
    while (1) {
        char input[max_length];
        uart_read_line(input);
        check(input);
    }
}
