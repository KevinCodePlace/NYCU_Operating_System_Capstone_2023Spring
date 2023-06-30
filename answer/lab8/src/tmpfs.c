#include "tmpfs.h"
#include "vfs.h"
#include "utils_c.h"
#include "mm.h"
#include "stat.h"
#include "mini_uart.h"

static int lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
static int create(struct vnode *dir_node, struct vnode **target, const char *component_name);
static int mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
struct vnode_operations tmpfs_v_ops = {
    lookup,
    create,
    mkdir,
};

static int write(struct file *file, const void *buf, size_t len);
static int read(struct file *file, void *buf, size_t len);
static int open(struct vnode *file_node, struct file **target);
static int close(struct file *file);
static long lseek64(struct file *file, long offset, int whence);
struct file_operations tmpfs_f_ops = {
    write,
    read,
    open,
    close,
    lseek64,
};

static int setup_mount(struct filesystem *fs, struct mount *mount)
{
    //  should set mount->root as a obj before call setup_mount
    mount->root->mount = mount;
    mount->root->f_ops = &tmpfs_f_ops;
    mount->root->v_ops = &tmpfs_v_ops;
    mount->fs = fs;

    // clear the internal content
    list_init(&mount->root->childs);
    mount->root->child_num = 0;
    mount->root->content = NULL;
    mount->root->content_size = 0;

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
static int mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    if (!lookup(dir_node, target, component_name))
    {
        uart_printf("[mkdir] the %s directory is already exist\n", component_name);
        return -1;
    }

#ifdef FS_DEBUG
    uart_printf("[fs] mkdir:%s, parent:%s\n", component_name, dir_node->name);
#endif

    struct vnode *new_vnode = vnode_create(component_name, S_IFDIR);
    new_vnode->mount = dir_node->mount;
    new_vnode->v_ops = dir_node->v_ops;
    new_vnode->f_ops = dir_node->f_ops;
    new_vnode->parent = dir_node;

    insert_tail(&dir_node->childs, &new_vnode->self);
    dir_node->child_num += 1;

    *target = new_vnode;
    return 0;
}

static int write(struct file *file, const void *buf, size_t len)
{
    struct vnode *vnode = file->vnode;
    if (!S_ISREG(vnode->f_mode))
    {
        uart_send_string("[write] not a regular file\n");
        return -1;
    }
    if (vnode->content_size <= file->f_pos + len)
    { // enlarge content
        void *new_content = kcalloc(sizeof(file->f_ops + len ));
        memcpy(new_content, vnode->content, vnode->content_size); // origin data;
        if (vnode->content)
        { // avoid the free the 0 in beginning
            kfree(vnode->content);
        }

        vnode->content = new_content;
        vnode->content_size = file->f_pos + len ; // pos=22  len=8  30
    }

    memcpy(vnode->content + file->f_pos, buf, len);
    file->f_pos += len;

    return len;
}
static int read(struct file *file, void *buf, size_t len)
{
    struct vnode *vnode = file->vnode;
    if (!S_ISREG(vnode->f_mode))
    {
        uart_send_string("[read] not a regular file\n");
        return -1;
    }
    int min = (len > vnode->content_size - file->f_pos ) ? vnode->content_size - file->f_pos : len; // -1 for EOF;
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

struct filesystem *tmpfs_create()
{
    struct filesystem *fs = kmalloc(sizeof(struct filesystem));
    fs->name = "tmpfs";
    fs->setup_mount = &setup_mount;
    list_init(&fs->list);
    return fs;
}
