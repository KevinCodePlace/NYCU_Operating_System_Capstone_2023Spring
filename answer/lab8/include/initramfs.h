#ifndef __INITRAM_FS_H
#define __INITRAM_FS_H

struct filesystem *initramfs_create();

extern struct file_operations initramfs_f_ops;
extern struct vnode_operations initramfs_v_ops;

#endif
