#pragma once
#define size_t unsigned long
#include "list.h"
#define O_CREAT  00000100
#define SEEK_SET 0

struct vnode {
  struct mount* mount;
  struct vnode_operations* v_ops;
  struct file_operations* f_ops;
  struct dentry* dt;
  size_t size;
  void* internal;
};

struct dentry {
    char name[32];
    struct dentry* parent;
    struct link_list* childs;
    struct vnode* vnode;
    enum type{
      directory,
      file,
      device
    }type;
};

// file handle
struct file {
  struct vnode* vnode;
  size_t f_pos;  // RW position of this file handle
  struct file_operations* f_ops;
  int flags;
};

struct mount {
  struct vnode* root;
  struct filesystem* fs;
};

struct filesystem {
  const char* name;
  int (*setup_mount)(struct filesystem* fs, struct mount* mount);
};

struct device {
  const char* name;
  int (*setup)(struct device* device, struct vnode* mount);
};

struct file_operations {
  int (*write)(struct file* file, const void* buf, size_t len);
  int (*read)(struct file* file, void* buf, size_t len);
  int (*open)(struct vnode* file_node, struct file** target);
  int (*close)(struct file* file);
  long (*lseek64)(struct file* file, long offset, int whence);
};

struct vnode_operations {
  int (*lookup)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*create)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*mkdir)(struct vnode* dir_node, struct vnode** target,
              const char* component_name);
  int (*mknod)(struct vnode* dir_node, struct vnode** target,
              const char* component_name);
  int (*ioctl)(struct file* file, unsigned long request, ...);
};

void vfs_init();
int register_filesystem(struct filesystem* fs);
int register_device(struct device* d);
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, size_t len);
int vfs_read(struct file* file, void* buf, size_t len);
int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);
int vfs_lookup(const char* pathname, struct vnode** target);
void vfs_ls(const char* pathname);
void vfs_cd(const char* pathname);
struct dentry* allo_dentry();
struct vnode* allo_vnode();
int vfs_mknod(const char* pathname, const char* device);
long vfs_lseek(struct file* f, long offset, int whence);

