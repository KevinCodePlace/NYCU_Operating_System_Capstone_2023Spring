#include "initramfs.h"
#include "vfs.h"
#include "utils_c.h"
#include "mm.h"
#include "stat.h"
#include "mini_uart.h"
#include "_cpio.h"
#include "dtb.h"

static int lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
static int create(struct vnode *dir_node, struct vnode **target, const char *component_name);
static int mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
struct vnode_operations initramfs_v_ops = {
    lookup,
    create,
    mkdir,
};

static int write(struct file *file, const void *buf, size_t len);
static int read(struct file *file, void *buf, size_t len);
static int open(struct vnode *file_node, struct file **target);
static int close(struct file *file);
static long lseek64(struct file *file, long offset, int whence);
struct file_operations initramfs_f_ops = {
    write,
    read,
    open,
    close,
    lseek64,
};

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

static int create_init(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    if (!lookup(dir_node, target, component_name))
    {
        uart_printf("[create] the %s file is already exist\n", component_name);
        return -1;
    }

#ifdef FS_DEBUG
    uart_printf("[fs] create file:%s, parent:%s\n", component_name, dir_node->name);
#endif

    struct vnode *new_vnode = vnode_create(component_name, S_IFREG);
    new_vnode->mount = dir_node->mount;
    new_vnode->v_ops = dir_node->v_ops;
    new_vnode->f_ops = dir_node->f_ops;
    new_vnode->parent = dir_node;

    insert_tail(&dir_node->childs, &new_vnode->self);
    dir_node->child_num += 1;

    *target = new_vnode;
    return 0;
}

static void init_cpio_files()
{
    struct vnode *dir = NULL;
    struct vnode *target = NULL;
    if (vfs_lookup("/initramfs", &dir))
    {
        uart_printf("[init_cpio_files] fail to lookup dir\n");
    }

    char *addr = initramfs_start;
    while (utils_str_compare((char *)(addr + sizeof(cpio_header)), "TRAILER!!!") != 0)
    {
        cpio_header *header = (cpio_header *)addr;
        unsigned int pathname_size = hex2dec(header->c_namesize);
        unsigned int file_size = hex2dec(header->c_filesize);
        unsigned int headerPathname_size = sizeof(cpio_header) + pathname_size;

        align(&headerPathname_size, 4);
        align(&file_size, 4);

        create_init(dir, &target, (char *)(addr + sizeof(cpio_header)));
        target->content = addr + headerPathname_size;
        target->content_size = file_size;

        addr += (headerPathname_size + file_size);
    }
}

static int setup_mount(struct filesystem *fs, struct mount *mount)
{
    //  should set mount->root as a obj before call setup_mount
    mount->root->mount = mount;
    mount->root->f_ops = &initramfs_f_ops;
    mount->root->v_ops = &initramfs_v_ops;
    mount->fs = fs;

    // clear the internal content
    list_init(&mount->root->childs);
    mount->root->child_num = 0;
    mount->root->content = NULL;
    mount->root->content_size = 0;
    init_cpio_files();
    return 0;
}

static int lookup(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
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
    return -1;
}
static int mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    return -1;
}

static int write(struct file *file, const void *buf, size_t len)
{
    return -1;
}
static int read(struct file *file, void *buf, size_t len)
{
    struct vnode *vnode = file->vnode;
    if (!S_ISREG(vnode->f_mode))
    {
        uart_send_string("[write] not a regular file\n");
        return -1;
    }
    int min = (len > vnode->content_size - file->f_pos - 1) ? vnode->content_size - file->f_pos - 1 : len; // -1 for EOF;
    if (min == 0)
    {
        return -1; // f_pos at EOF or len==0;
    }
    memcpy(buf, vnode->content + file->f_pos, min);
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

struct filesystem *initramfs_create()
{
    struct filesystem *fs = kmalloc(sizeof(struct filesystem));
    fs->name = "initramfs";
    fs->setup_mount = &setup_mount;
    list_init(&fs->list);
    return fs;
}
