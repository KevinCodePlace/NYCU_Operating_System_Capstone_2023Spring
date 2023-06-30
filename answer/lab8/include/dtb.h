#ifndef _DTB_H
#define _DTB_H
#include <stdint.h>
#include <stddef.h>
/*
    structure block: located at a 4-byte aligned offset from the beginning of the devicetree blob
    token is a big-endian 32-bit integer, alligned on 32bit(padding 0)

*/

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

typedef struct fdt_header
{
    uint32_t magic;         // contain the value 0xd00dfeed (big-endian).
    uint32_t totalsize;     // in byte
    uint32_t off_dt_struct; // the offset in bytes of the structure block from the beginning of the header
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings; // the length in bytes of the strings block section
    uint32_t size_dt_struct;
} fdt_header;

typedef void (*fdt_callback)(int type, const char *name, const void *data, uint32_t size);
void print_dtb(int type, const char *name, const void *data, uint32_t size);
void get_initramfs_addr(int type, const char *name, const void *data, uint32_t size);
int fdt_traverse(fdt_callback cb);

extern uintptr_t dtb_end, dtb_start;
extern char *initramfs_start, *initramfs_end;

#endif