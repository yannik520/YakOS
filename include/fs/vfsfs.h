/*
 * Copyright (c) 2013 Yannik Li(Yanqing Li)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __VFSFS_H__
#define __VFSFS_H__

#include <kernel/list.h>

#ifdef VFS_TEST
#define VFS_MALLOC(t, n) t *n = (t *)malloc(sizeof(t))
#define printk printf
#define kmalloc malloc
#define kfree free
#else
#include <kernel/printk.h>
#include <kernel/malloc.h>
#define VFS_MALLOC(t, n) t *n = (t *)kmalloc(sizeof(t))
#endif

#define VFS_ASSERT(x) do {						\
		if (!(x)) {						\
			printk("%s %d Assertion failed: %s\n", __FILE__, __LINE__, #x); \
			return -1;					\
		}							\
	}while(0)

struct vfs_node;

struct vfs_opvector {
    int (*lookup)(void *, const char *, struct vfs_node *);
    int (*read)(void *, void *, size_t, size_t *);
    int (*close)(void *);
    int (*open)(void *);
};

struct vfs_node {
    struct vfs_opvector *vops;
    void *priv;
};

struct vfs_fs {
  	char	name[32];
	struct list_head list;
#ifdef VFS_TEST
  	int (*mount)(const char *image, struct vfs_node *vfsroot);
#else
	int (*mount)(const uint8_t *image, struct vfs_node *vfsroot);
#endif
};

#define FD_MAX_NUM	256
#define FD_ALLOCATED	((struct vfs_node *)1)

int register_filesystem(struct vfs_fs *fs);

#ifdef VFS_TEST
int vfs_mount(const char *image, const char *name);
#else
int vfs_mount(const uint8_t *image, const char *name);
#endif
int vfs_lookup(struct vfs_node *dir, const char *name, struct vfs_node *namei);

int vfs_opendir(const char *path, struct vfs_node *dir);
int vfs_closedir(struct vfs_node *dir);

int vfs_open(const char *path, struct vfs_node *file);
int vfs_read(int fd, void *buf, size_t count, size_t *ready);
int vfs_close(int fd);

#endif

