#include "dtb.h"
#include "_cpio.h"
#include "mini_uart.h"
#include "utils_c.h"
/*
    It consists of
    a small header +
    the memory reservation block + space(aligned) +
    the structure block + space(aligned) +
    the strings block + space(aligned)
*/
int space = 0;
char * cpio_addr;

uint32_t get_le2be_uint(const void *p)
{
    // transfer little endian to big endian
    const unsigned char *bytes = p;
    uint32_t res = bytes[3];
    res |= bytes[2] << 8;
    res |= bytes[1] << 16;
    res |= bytes[0] << 24;
    return res;
}

void send_sapce(int n) {
  while (n--) uart_send_string(" ");
}

int parse_struct(fdt_callback cb, uintptr_t cur_ptr, uintptr_t strings_ptr, uint32_t totalsize)
{
    uintptr_t end_ptr = cur_ptr + totalsize;

    while (cur_ptr < end_ptr)
    {
        uint32_t token = get_le2be_uint((char *)cur_ptr);
        cur_ptr += 4;
        switch (token)
        {
            case FDT_BEGIN_NODE:
                // uart_send_string("In FDT_BEGIN_NODE\n");
                cb(token, (char *)cur_ptr, NULL, 0);
                cur_ptr += align_up(utils_strlen((char *)cur_ptr), 4);   
                break;
            case FDT_END_NODE:
                // uart_send_string("In FDT_END_NODE\n");
                cb(token, NULL, NULL, 0);
                break;
            case FDT_PROP:{
                // uart_send_string("In FDT_PROP\n");
                uint32_t len = get_le2be_uint((char *)cur_ptr);
                cur_ptr += 4;
                uint32_t nameoff = get_le2be_uint((char *)cur_ptr);
                cur_ptr += 4;
                cb(token, (char *)(strings_ptr + nameoff), (void *)cur_ptr, len);
                cur_ptr +=  align_up(len, 4);;
                break;
            }
            case FDT_NOP:
                // uart_send_string("In FDT_NOP\n");
                cb(token, NULL, NULL, 0);
                break;
            case FDT_END:
                // uart_send_string("In FDT_END\n");
                cb(token, NULL, NULL, 0);
                return 0;
            default:;
                return -1;
        }
    }
    return 0;
}



void print_dtb(int type, const char *name, const void *data, uint32_t size)
{
    switch (type)
    {
    case FDT_BEGIN_NODE:
        uart_send_string("\n");
        send_sapce(space);
        uart_send_string(name);
        uart_send_string("{\n ");
        space++;
        break;

    case FDT_END_NODE:
        uart_send_string("\n");
        space--;
        if (space>0) send_sapce(space);
        
        uart_send_string("}\n");
        break;

    case FDT_NOP:
        break;

    case FDT_PROP:
        send_sapce(space);
        uart_send_string(name);
        break;

    case FDT_END:
        break;
    }
}

void get_initramfs_addr(int type, const char *name, const void *data, uint32_t size)
{
    if(type==FDT_PROP&&!utils_str_compare(name,"linux,initrd-start")){
        cpio_addr=(char *)(uintptr_t)get_le2be_uint(data);
        uart_send_string("initramfs_addr at ");
        uart_hex((uintptr_t)get_le2be_uint(data));
        uart_send('\n');
    }
}

int fdt_traverse(fdt_callback cb, void* _dtb)
{
    uintptr_t dtb_ptr = (uintptr_t)_dtb;
    uart_send_string("\ndtb loading at:");
    uart_hex(dtb_ptr);
    uart_send('\n');
    fdt_header *header = (fdt_header *)dtb_ptr;

    if (get_le2be_uint(&(header->magic)) != 0xd00dfeed)
    {
        uart_send_string("header magic != 0xd00dfeed\n");
        return -1;
    }
    uint32_t totalsize = get_le2be_uint(&(header->totalsize));
    uintptr_t struct_ptr = dtb_ptr + get_le2be_uint(&(header->off_dt_struct));
    uintptr_t strings_ptr = dtb_ptr + get_le2be_uint(&(header->off_dt_strings));
    parse_struct(cb, struct_ptr, strings_ptr, totalsize);
    return 0;
}
