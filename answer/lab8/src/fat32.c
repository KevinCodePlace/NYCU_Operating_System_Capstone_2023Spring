#include "fat32.h"
#include "vfs.h"
#include "mm.h"
#include "sd.h"
#include "utils_c.h"
#include "mini_uart.h"
#include "list.h"

/* ref : https://www.pjrc.com/tech/8051/ide/fat32.html */

#define MSDOS_NAME 11 /* maximum name length */
#define BLOCK_SIZE 512
#define FILENAME_SIZE MSDOS_NAME + 2
#define DIR_SIZE 16
#define DIR_ENT_UNUSED 0xE5
#define END_OF_CHAIN_END 0xFFFFFFF
#define END_OF_CHAIN_START 0xFFFFFF8

struct fat_internal
{
    unsigned int cluster_id;
    unsigned int cluster_len;
    unsigned int is_dirty;
};

struct fat_boot_sector metadata;
unsigned int *FAT_table = NULL;
unsigned int *table_dirty_record = NULL;

/* size in sectors*/
unsigned int fat_region_start;  /* file allocation table #1 #2 ... */
unsigned int data_region_start; /* files and directories ... */

static int lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
static int create(struct vnode *dir_node, struct vnode **target, const char *component_name);
static int mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
struct vnode_operations fat32_v_ops = {
    lookup,
    create,
    mkdir,
};

static int write(struct file *file, const void *buf, size_t len);
static int read(struct file *file, void *buf, size_t len);
static int open(struct vnode *file_node, struct file **target);
static int close(struct file *file);
static long lseek64(struct file *file, long offset, int whence);
struct file_operations fat32_f_ops = {
    write,
    read,
    open,
    close,
    lseek64,
};

static int parse_mbr()
{
    struct mbr mbr;

    readblock(0, &mbr);
    if (mbr.signature[0] != 0x55 || mbr.signature[1] != 0xAA)
    {
        uart_printf("[parse_mbr] sig not match\n");
        return -1;
    }

    /*
    if (mbr.pe[0].status != 0x80)
    {
        uart_printf("[parse_mbr] status not match\n");
        return -1;
    }
    */

    /* FAT32 with CHS addressing:0xb, FAT32 with LBA:0xc */
    if (mbr.pe[0].type != 0xb && mbr.pe[0].type != 0xc)
    {
        uart_printf("[parse_mbr] type not match\n");
        return -1;
    }

    /* FAT32 store its metadata in the first sector of the partition */
    unsigned int boot_sec_idx = mbr.pe[0].first_sector;
    readblock(boot_sec_idx, (void *)&metadata); // valid data range is 0~446 bytes

    fat_region_start = boot_sec_idx + metadata.n_reserved_sec;
    data_region_start = fat_region_start + metadata.n_fats * metadata.sec_per_fat32 - 2;

    return 0;
}

unsigned int get_chain_len(int id)
{
    int ret = 1;
    while (1)
    {
        id = FAT_table[id];
        if (id >= 2 && id <= 0xFFFFFEF)
        {
            ret++;
        }
        else if (id >= END_OF_CHAIN_START && id <= END_OF_CHAIN_END)
        {
            break;
        }
        else
        {
            uart_printf("[get_chain_len] unknown FAT value\n");
            return -1;
        }
    }
    return ret;
}
unsigned int get_chain(unsigned int id, unsigned char **buf)
{
    int chain_len = get_chain_len(id);
    unsigned int len = chain_len * metadata.byte_per_sec;
    *buf = kcalloc(len);
    for (int i = 0; i < len; i += metadata.byte_per_sec)
    {
        readblock(data_region_start + id, (*buf) + i);
        id = FAT_table[id];
    }
    return len;
}

void init_node(struct vnode *node)
{
    /* set the file content to node from cluster */
    unsigned char *buf;
    struct fat_internal *internal = (struct fat_internal *)node->internal;
    internal->cluster_len = get_chain(internal->cluster_id, &buf);
    node->content = buf;
    internal->is_dirty = 0;
}

void init_root(struct vnode *node)
{
    int sector_size = metadata.byte_per_sec;
    unsigned char *table = kmalloc(metadata.n_fats * metadata.sec_per_fat32 * sector_size); // byte_per_sec will be 512 byte;

    for (int i = 0; i < metadata.sec_per_fat32 * metadata.n_fats; i++)
    {
        readblock(fat_region_start + i, table + i * sector_size);
    }
    if (FAT_table)
    {
        uart_printf("[init_node] dirty table\n");
        return;
    }
    FAT_table = (unsigned int *)table;
    table_dirty_record = kcalloc(metadata.n_fats * metadata.sec_per_fat32 * sizeof(unsigned int));
    for (int i = 0; i < metadata.n_fats * metadata.sec_per_fat32; i++)
    {
        table_dirty_record[i] = 0;
    }

    node->internal = kcalloc(sizeof(struct fat_internal));
    struct fat_internal *internal = (struct fat_internal *)node->internal;

    internal->cluster_id = 2;

    init_node(node);
    internal->is_dirty = 1;
}

int parse_dir_entry(struct fat_dir *dir_ent, struct vnode *node, struct vnode *parent)
{
    if (dir_ent->attr == 0xf)
    {
        // ignore LFN
        uart_printf("[parse_dir_entry] encounter LFN\n");
        return -1;
    }

    unsigned char tmp_name[FILENAME_SIZE];
    int idx = 0;
    for (int i = 0; i < 8; i++)
    {
        if (dir_ent->name[i] == ' ')
        {
            break;
        }
        tmp_name[idx++] = dir_ent->name[i];
    }

    int add_dot = 0;

    for (int i = 0; i < 3; i++)
    {
        if (dir_ent->ext[i] == ' ')
        {
            break;
        }
        if (!add_dot)
        {
            tmp_name[idx++] = '.';
            add_dot = 1;
        }
        tmp_name[idx++] = dir_ent->ext[i];
    }
    tmp_name[idx] = '\0';

    unsigned int cluster_id = (dir_ent->clus_hi << 16) + dir_ent->clus_lo;
    unsigned int f_mode;
    if (dir_ent->attr & ATTR_ARCHIVE)
    {
        f_mode = S_IFREG;
    }
    else
    {
        f_mode = S_IFDIR;
    }

    node->mount = parent->mount;
    node->f_ops = parent->f_ops;
    node->v_ops = parent->v_ops;
    node->parent = parent;
    node->content_size = dir_ent->file_size;
    memcpy(node->name, tmp_name, FILENAME_SIZE);
    node->f_mode = f_mode;

    node->internal = kmalloc(sizeof(struct fat_internal));
    struct fat_internal *internal = node->internal;

    internal->cluster_id = cluster_id;
    internal->is_dirty = 0;
    return 0;
}

static int setup_mount(struct filesystem *fs, struct mount *mount)
{
    //  should set mount->root as a obj before call setup_mount
    mount->root->mount = mount;
    mount->root->f_ops = &fat32_f_ops;
    mount->root->v_ops = &fat32_v_ops;
    mount->fs = fs;

    // clear the internal content
    list_init(&mount->root->childs);
    mount->root->child_num = 0;
    mount->root->content = NULL;
    mount->root->content_size = 0;

    parse_mbr();
    init_root(mount->root);

    return 0;
}
static struct vnode *fat_node_create()
{
    struct vnode *vnode = kcalloc(sizeof(struct vnode));
    list_init(&vnode->childs);
    list_init(&vnode->self);
    vnode->child_num = 0;
    vnode->parent = NULL;
    vnode->f_mode = 0;
    vnode->content = NULL;
    vnode->content_size = 0;
    vnode->name = kmalloc(FILENAME_SIZE);
    vnode->internal = NULL;
    return vnode;
}

static int sync(struct vnode *node)
{
    struct fat_internal *internal = (struct fat_internal *)node->internal;
    struct vnode *parent = node->parent;

    if (!node->content || !internal->is_dirty)
    {
        return -1;
    }

    /* update FAT_table*/
    int old_block = get_chain_len(internal->cluster_id);
    int new_block = align_up(node->content_size, BLOCK_SIZE) / BLOCK_SIZE;
    if (old_block > new_block)
    {
        uart_printf("[sync] old_block>new_block!\n");
        return -1;
    }

    int last_id = internal->cluster_id;
    while (FAT_table[last_id] < END_OF_CHAIN_START)
    {
        last_id = FAT_table[last_id];
    }

    for (int i = 0; metadata.n_fats * metadata.sec_per_fat32 * metadata.byte_per_sec / sizeof(unsigned int) && old_block < new_block; i++)
    {
        if (FAT_table[i] != 0)
        {
            continue;
        }
        FAT_table[last_id] = i;
        table_dirty_record[last_id / BLOCK_SIZE] = 1;
        last_id = i;
        FAT_table[last_id] = END_OF_CHAIN_START;
        table_dirty_record[last_id / BLOCK_SIZE] = 1;
        old_block++;
    }

    for (int i = 0; i < metadata.n_fats * metadata.sec_per_fat32; i++)
    {
        if (table_dirty_record[i])
        {
            writeblock(fat_region_start + i, ((char *)FAT_table) + BLOCK_SIZE * i);
            table_dirty_record[i] = 0;
        }
    }
    // update data
    unsigned char *data = (unsigned char *)node->content;
    int cur_id = internal->cluster_id;
    for (int i = 0; i < node->content_size; i += BLOCK_SIZE)
    {
        if (cur_id < 2 || cur_id > 0xFFFFFEF)
        {
            uart_printf("[sync] Invalid table id\n");
            return -1;
        }
        writeblock(data_region_start + cur_id, data);
        data += BLOCK_SIZE;
        cur_id = FAT_table[cur_id];
    }

    /* update parent */
    int new_file_idx = -1, idx = 0;
    struct vnode *vnode = NULL;
    list_for_each_entry(vnode, &parent->childs, self)
    {
        if (!(utils_str_compare(vnode->name, node->name)))
        {
            new_file_idx = idx;
            break;
        }
        idx++;
    }
    if (new_file_idx == -1)
    {
        uart_printf("[sync] sync an unknown file\n");
        return -1;
    }
    cur_id = ((struct fat_internal *)(parent->internal))->cluster_id;
    for (int i = 0; i < DIR_SIZE; i++)
    {
        if (FAT_table[cur_id] >= END_OF_CHAIN_START && FAT_table[cur_id] <= END_OF_CHAIN_END)
        {
            break;
        }
        cur_id = FAT_table[cur_id];
    }

    char buf[BLOCK_SIZE];
    readblock(data_region_start + cur_id, buf);
    struct fat_dir *dir_ent = (struct fat_dir *)(buf + new_file_idx % DIR_SIZE * sizeof(struct fat_dir));
    dir_ent->file_size = node->content_size;
    writeblock(data_region_start + cur_id, buf);

    internal->is_dirty = 0;
    return 0;
}

static int get_free_cluster_id(struct vnode *dir_node)
{
    struct fat_internal *dir_internal = (struct fat_internal *)dir_node->internal;
    unsigned char *buf;
    unsigned int len = get_chain(dir_internal->cluster_id, &buf);
    for (int i = 0; i < len; i += 32)
    {
        if (buf[i] == 0)
        {
            return i / 32;
        }
    }
    return 0;
}

static int lookup(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    /* create the vnode instance in memory from sdcard */
    struct fat_internal *internal = (struct fat_internal *)dir_node->internal;

    if (internal->is_dirty)
    {
        dir_node->child_num = 0;
        unsigned char *buf;
        unsigned int len = get_chain(internal->cluster_id, &buf);
        list_init(&dir_node->childs);
        for (int i = 0; i < len; i += 32)
        {
            if (buf[i] == 0)
            {
                break;
            }
            struct vnode *node = fat_node_create();
            if (parse_dir_entry((struct fat_dir *)(buf + i), node, dir_node) == -1)
            {
                continue;
            };
            insert_tail(&dir_node->childs, &node->self);
            dir_node->child_num++;
        }
        internal->is_dirty = 0;
    }

    struct vnode *vnode = NULL;
    list_for_each_entry(vnode, &dir_node->childs, self)
    {
        if (!(utils_str_compare(vnode->name, component_name)))
        {
            *target = vnode;
            return 0;
        }
    }
    return -1;
}
static int create(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    if (!lookup(dir_node, target, component_name))
    {
        uart_printf("[create] the %s file is already exist\n", component_name);
        return -1;
    }

    struct fat_internal *dir_internal = dir_node->internal;
    int cluster_id = get_free_cluster_id(dir_node);

    /* update for firmware */
    char buf[BLOCK_SIZE];
    struct fat_dir *new_ent = NULL;

    readblock(data_region_start + dir_internal->cluster_id, buf);
    for (int child_idx = 0; child_idx < BLOCK_SIZE; child_idx += 32)
    {
        unsigned char *flag = (unsigned char *)&buf[child_idx];
        if (*flag == DIR_ENT_UNUSED || *flag == 0)
        {
            new_ent = (struct fat_dir *)&buf[child_idx];
            int idx = 0;
            for (int i = 0; i < 8; i++)
            {
                new_ent->name[i] = ' '; /* the unused bytes must filled with spaces (0x20) */
            }
            for (int i = 0; i < 8; i++)
            {
                new_ent->name[idx] = component_name[idx];
                idx++;
                if (component_name[idx] == '.')
                {
                    break;
                }
            }
            idx++;
            if (component_name[idx])
            {
                for (int i = 0; i < 3; i++)
                {
                    new_ent->ext[i] = component_name[idx];
                    idx++;
                }
            }
            new_ent->attr = ATTR_ARCHIVE;
            new_ent->file_size = 0;
            new_ent->clus_hi = cluster_id & (0xffff0000);
            new_ent->clus_lo = cluster_id & (0x0000ffff);

            writeblock(data_region_start + dir_internal->cluster_id, buf);
            break;
        }
    }
    if (!new_ent)
    {
        uart_printf("[create] fail to writeback\n");
        return -1;
    }

    /* update for mem */
    struct vnode *node = fat_node_create();
    struct fat_internal *internal = node->internal;

    node->mount = dir_node->mount;
    node->f_ops = dir_node->f_ops;
    node->v_ops = dir_node->v_ops;
    node->parent = dir_node;
    node->content_size = 0;
    memcpy(node->name, component_name, FILENAME_SIZE);
    node->f_mode = S_IFREG;

    internal->is_dirty = 1;
    dir_internal->is_dirty = 1;
    internal->cluster_id = cluster_id;

    insert_tail(&dir_node->childs, &node->self);
    dir_node->child_num += 1;

    FAT_table[internal->cluster_id] = END_OF_CHAIN_END;

    *target = node;

    return 0;
}
static int mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    // TODO
    return 0;
}

static int write(struct file *file, const void *buf, size_t len)
{
    struct vnode *node = file->vnode;
    struct fat_internal *internal = node->internal;

    if (!S_ISREG(node->f_mode))
    {
        uart_send_string("[write] not a regular file\n");
        return -1;
    }
    init_node(node);

    if (node->content_size <= file->f_pos + len)
    { // enlarge content
        void *new_content = kcalloc(sizeof(file->f_ops + len));
        memcpy(new_content, node->content, node->content_size); // origin data;
        if (node->content)
        { // avoid the free the 0 in beginning
            kfree(node->content);
        }
        node->content = new_content;
        node->content_size = file->f_pos + len; // pos=22  len=8  30
    }
    memcpy(node->content + file->f_pos, buf, len);
    file->f_pos += len;

    internal->is_dirty = 1;
    sync(node);
    return len;
}
static int read(struct file *file, void *buf, size_t len)
{
    struct vnode *node = file->vnode;
    if (!S_ISREG(node->f_mode))
    {
        uart_send_string("[read] not a regular file\n");
        return -1;
    }
    int min = (len > node->content_size - file->f_pos) ? node->content_size - file->f_pos : len;
    if (min == 0)
    {
        return -1; // f_pos at EOF or len==0;
    }

    init_node(node);

    memcpy(buf, node->content + file->f_pos, min);
    file->f_pos += min;

    return min;
}
static int open(struct vnode *file_node, struct file **target)
{
    // TODO
    return 0;
}
static int close(struct file *file)
{
    kfree(file);
    return 0;
}
static long lseek64(struct file *file, long offset, int whence)
{
    // TODO
    return 0;
}

struct filesystem *fat32_create()
{
    struct filesystem *fs = kmalloc(sizeof(struct filesystem));
    fs->name = "fat32";
    fs->setup_mount = &setup_mount;
    list_init(&fs->list);
    return fs;
}
