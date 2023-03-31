#include "header/dtb.h"
int raN(){
	return 0;
}
/*
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned char uint8_t;
typedef unsigned long my_size_t;


//1. Define the callback function type:

typedef void (*fdt_callback_t)(const void *node, void *user_data);

//2.Create a structure for holding the FDT header information:
struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t dt_strings_size;
    uint32_t dt_struct_size;
}

3. Implement helper functions to extract the FDT header information:

uint32_t fdt_read_u32(const void *addr) {
    const uint8_t *bytes = (const uint8_t *)addr;
    uint32_t val = (uint32_t)bytes[0] << 24 | (uint32_t)bytes[1] << 16 | (uint32_t)bytes[2] << 8 | (uint32_t)bytes[3];
    return val;
}

my_size_t my_strlen(const char *s) {
    my_size_t len = 0;
    while (*s) {
        len++;
        s++;
    }
    return len;
}

void fdt_extract_header(const void *fdt, fdt_header_t *header) {
    header->magic = fdt_read_u32(fdt);
    header->totalsize = fdt_read_u32(fdt + 4);
    header->off_dt_struct = fdt_read_u32(fdt + 8);
    header->off_dt_strings = fdt_read_u32(fdt + 12);
    header->off_mem_rsvmap = fdt_read_u32(fdt + 16);
    header->version = fdt_read_u32(fdt + 20);
    header->last_comp_version = fdt_read_u32(fdt + 24);
    header->boot_cpuid_phys = fdt_read_u32(fdt + 28);
    header->size_dt_strings = fdt_read_u32(fdt + 32);
    header->size_dt_struct = fdt_read_u32(fdt + 36);
}

//4. Implement the fdt_traverse function:

void fdt_traverse(const void *fdt, fdt_callback_t callback, void *user_data) {
    fdt_header_t header;
    fdt_extract_header(fdt, &header);

    const uint8_t *dt_struct = (const uint8_t *)fdt + header.off_dt_struct;

    while (dt_struct < (const uint8_t *)fdt + header.off_dt_struct + header.size_dt_struct) {
        uint32_t token = fdt_read_u32(dt_struct);
        dt_struct += sizeof(uint32_t);

        switch (token) {
            case 0x00000001: // FDT_BEGIN_NODE
                // Process the node and call the callback function
                callback(dt_struct, user_data);

                // Move to the end of the node's name (including NULL terminator)
                dt_struct += my_strlen((const char *)dt_struct) + 1;
                break;

            case 0x00000002: // FDT_END_NODE
                // Continue to the next node
                break;

            case 0x00000003: // FDT_PROP
                // Move to the next property or node
                dt_struct += 3 * sizeof(uint32_t) + fdt_read_u32(dt_struct + sizeof(uint32_t));
                break;

            case 0x00000009: // FDT_END
                // End of the device tree
                return;

            default:
                // Invalid token or unsupported token
                return;
        }
    }
}

//5. Implement the initramfs_callback function:
void initramfs_callback(const void *node, void *user_data) {
    const char *node_name = (const char *)node;
    // Process the node_name and node properties as needed
    // for the initramfs_callback specific tasks
}
*/
