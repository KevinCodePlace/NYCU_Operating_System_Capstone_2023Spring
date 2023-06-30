#ifndef __FAT32_H_
#define __FAT32_H_

#include <stdint.h>
#include <stddef.h>
#define MSDOS_NAME 11 /* maximum name length */
#define BLOCK_SIZE 512

#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20

struct fat_boot_sector
{
    uint8_t jmp_boot[3];           /* boot strap short or near jump */
    uint8_t oem_name[8];           /* OEM Name - can be used to special case partition manager volumes */
    uint16_t byte_per_sec;         /* number of bytes per logical sector, must be one of 512,1024,2048,4096 */
    uint8_t sec_per_clus;          /* number of sectors per cluster, must be 1,2,4,8,16,32,61,128 */
    uint16_t n_reserved_sec;       /* reserved sectors */
    uint8_t n_fats;                /* number of FATs */
    uint16_t n_dir_entries;        /* number of root directory entries */
    uint16_t n_sectors;            /* number of sectors */
    uint8_t media;                 /* media descriptor type */
    uint16_t sec_per_fat16;        /* logical sectors per FAT16 */
    uint16_t sec_per_track;        /* sectors per track */
    uint16_t n_heads;              /* number of heads */
    uint32_t n_hidden_sec;         /* hidden sectors (unused) */
    uint32_t n_sec;                /* number of sectors in filesystem (if sectors == 0) */
    uint32_t sec_per_fat32;        /* logical sectors per FAT32 */
    uint16_t flags;                /* bit 8: fat mirroring, low 4: active fat */
    uint16_t version;              /* major, minor filesystem version */
    uint32_t root_cluster;         /* first cluster in root directory */
    uint16_t info_sector;          /* filesystem info sector number in FAT32 reserved area*/
    uint16_t backup_boot;          /* backup boot sector */
    uint16_t reserved[6];          /* Unused */
    uint8_t drive_number;          /* Physical drive number */
    uint8_t state;                 /* undocumented, but used for mount state. */
    uint8_t signature;             /* extended boot signature */
    uint32_t vol_id;               /* volume ID */
    uint8_t vol_label[MSDOS_NAME]; /* volume label */
    uint8_t fs_type[8];            /* file system type e.g. "FAT32    " */
} __attribute__((packed));

struct chs_addr
{
    uint8_t head;
    uint8_t sector;
    uint8_t cylinder;
} __attribute__((packed));

struct partition_entry
{
    uint8_t status;
    struct chs_addr first;
    uint8_t type;
    struct chs_addr last;
    uint32_t first_sector;
    uint32_t n_sector;
} __attribute__((packed));

struct mbr
{
    char bootstrap_code_area[446];
    struct partition_entry pe[4];
    char signature[2];
} __attribute__((packed));

struct fat_dir
{
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attr;
    uint8_t nt_res; /*Reserved for use by Windows NT*/
    uint8_t crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t clus_hi; /*High word of this entry’s first cluster number*/
    uint16_t wrt_time;    /*Time of last write*/
    uint16_t wrt_date;
    uint16_t clus_lo; /* Low word of this entry’s first cluster number. */
    uint32_t file_size;
} __attribute__((packed));

struct filesystem *fat32_create();

extern struct file_operations fat32_f_ops;
extern struct vnode_operations fat32_v_ops;
#endif