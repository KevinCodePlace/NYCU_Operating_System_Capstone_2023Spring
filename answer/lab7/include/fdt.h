#include "uint.h"

#ifndef fdt
typedef struct fdt_header {
    uint32 magic;
    uint32 totalsize;
    uint32 off_dt_struct;
    uint32 off_dt_strings;
    uint32 off_mem_rsvmap;
    uint32 version;
    uint32 last_comp_version;
    uint32 boot_cpuid_phys;
    uint32 size_dt_strings;
    uint32 size_dt_struct;
}fdt_header;

typedef struct fdt_reserve_entry {
    uint64 address;
    uint64 size;
}fdt_reserve_entry;

typedef struct FDT_PROP
{
    uint32 len;
    uint32 nameoff;
}FDT_PROP;

/*typedef struct FDT{
    fdt_header* fdt_h;
    struct device{
        char *name;
        struct fdt_prop{
            char *name;
            FDT_PROP *prop;
        }*prop;
    }*device;
}fdt;*/


#endif