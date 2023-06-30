#ifndef __VFS_H_
#define __VFS_H_

#include "stat.h"
#include "list.h"

// #define FS_DEBUG
#define FD_TABLE_SIZE 16
#define O_CREAT 00000100
struct vnode
{
    struct mount *mount;
    struct vnode_operations *v_ops;
    struct file_operations *f_ops;

    // common internal
    list childs;
    size_t child_num;
    list self;
    struct vnode *parent;
    char *name;
    unsigned int f_mode;

    void *content;
    size_t content_size;

    // customer internal
    void *internal;
};

// file handle
struct file
{
    struct vnode *vnode;
    size_t f_pos; // RW position of this file handle
    struct file_operations *f_ops;
    int flags;
};

struct mount
{
    struct vnode *root;
    struct filesystem *fs;
};

struct filesystem
{
    const char *name;
    int (*setup_mount)(struct filesystem *fs, struct mount *mount);
    list list;
};

struct file_operations
{
    int (*write)(struct file *file, const void *buf, size_t len);
    int (*read)(struct file *file, void *buf, size_t len);
    int (*open)(struct vnode *file_node, struct file **target);
    int (*close)(struct file *file);
    long (*lseek64)(struct file *file, long offset, int whence);
};

struct vnode_operations
{
    int (*lookup)(struct vnode *dir_node, struct vnode **target,
                  const char *component_name);
    int (*create)(struct vnode *dir_node, struct vnode **target,
                  const char *component_name);
    int (*mkdir)(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
};

void fs_init();

int fs_register(struct filesystem *fs);
struct filesystem *fs_get(const char *name);

int vfs_open(const char *pathname, int flags, struct file **target);
int vfs_close(struct file *file);
int vfs_write(struct file *file, const void *buf, size_t len);
int vfs_read(struct file *file, void *buf, size_t len);
int vfs_mkdir(const char *pathname);
int vfs_mount(const char *target, const char *filesystem);
int vfs_lookup(const char *pathname, struct vnode **target);
int vfs_chdir(const char *pathname);
struct vnode *vnode_create(const char *name, unsigned int flags);
void vfs_test();


extern struct mount *rootfs, *initramfs;
#endif