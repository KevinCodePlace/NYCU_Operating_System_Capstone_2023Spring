#include "_cpio.h"
#include "utils_c.h"
#include "dtb.h"
#include "mini_uart.h"
#include "timer.h"
#include "mm.h"
#include "mmu.h"

static unsigned int hex2dec(char *s)
{
    unsigned int r = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (s[i] >= '0' && s[i] <= '9')
        {
            r = r * 16 + s[i] - '0';
        }
        else
        {
            r = r * 16 + s[i] - 'A' + 10;
        }
    }
    return r;
}

char *findFile(const char *name)
{
    char *addr = initramfs_start;
    while (utils_str_compare((char *)(addr + sizeof(cpio_header)), "TRAILER!!!") != 0)
    {
        if ((utils_str_compare((char *)(addr + sizeof(cpio_header)), name) == 0))
        {
            return addr;
        }
        cpio_header *header = (cpio_header *)addr;
        unsigned int pathname_size = hex2dec(header->c_namesize);
        unsigned int file_size = hex2dec(header->c_filesize);
        unsigned int headerPathname_size = sizeof(cpio_header) + pathname_size;

        align(&headerPathname_size, 4);
        align(&file_size, 4);
        addr += (headerPathname_size + file_size);
    }
    return 0;
}
void cpio_ls()
{
    char *addr = initramfs_start;
    while (utils_str_compare((char *)(addr + sizeof(cpio_header)), "TRAILER!!!") != 0)
    {
        cpio_header *header = (cpio_header *)addr;
        unsigned int pathname_size = hex2dec(header->c_namesize);
        unsigned int file_size = hex2dec(header->c_filesize);
        unsigned int headerPathname_size = sizeof(cpio_header) + pathname_size;

        align(&headerPathname_size, 4);
        align(&file_size, 4);

        uart_send_string(addr + sizeof(cpio_header)); // print the file name
        uart_send_string("\n");

        addr += (headerPathname_size + file_size);
    }
}

void cpio_cat(const char *filename)
{
    char *target = findFile(filename);
    if (target)
    {
        cpio_header *header = (cpio_header *)target;
        unsigned int pathname_size = hex2dec(header->c_namesize);
        unsigned int file_size = hex2dec(header->c_filesize);
        unsigned int headerPathname_size = sizeof(cpio_header) + pathname_size;

        align(&headerPathname_size, 4);
        align(&file_size, 4);

        char *file_content = target + headerPathname_size;
        for (unsigned int i = 0; i < file_size; i++)
        {
            uart_send(file_content[i]); // print the file content
        }
        uart_send_string("\n");
    }
    else
    {
        uart_send_string("Not found the file\n");
    }
}

size_t cpio_load_program(const char *filename, void **target_addr, pd_t *table)
{
    char *prog_addr = findFile(filename);
    if (prog_addr)
    {
        cpio_header *header = (cpio_header *)prog_addr;
        unsigned int pathname_size = hex2dec(header->c_namesize);
        unsigned int file_size = hex2dec(header->c_filesize);
        unsigned int headerPathname_size = sizeof(cpio_header) + pathname_size;

        align(&headerPathname_size, 4);
        align(&file_size, 4);

        uart_printf("load the %s\n", prog_addr + sizeof(cpio_header));

        char *file_content = prog_addr + headerPathname_size;
        *target_addr = alloc_prog(table, file_size, file_content);

        return file_size;
    }
    else
    {
        uart_send_string("Not found the program\n");
        return -1;
    }
}
