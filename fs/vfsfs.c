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

#ifdef VFS_TEST
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#endif

#include <string.h>
#include <fs/vfsfs.h>
#include <kernel/list.h>

LIST_HEAD(fs_root);
static struct vfs_node	__vfsroot;
static struct vfs_node *desc[FD_MAX_NUM] = {0};
static char cur_path[128]		 = "/";

static struct vfs_fs *find_filesystem(const char *name)
{
	struct vfs_fs *p;

	list_for_each_entry(p, &fs_root, list) {
		if (strncmp(p->name, name, strlen(name)) == 0)
			return p;
	}
	return NULL;
}

int register_filesystem(struct vfs_fs *fs)
{
	if (find_filesystem(fs->name)) return -1;

	list_add_tail(&fs->list, &fs_root);

	return 0;
}

#ifdef VFS_TEST
int vfs_mount(const char *image, const char *name)
#else
int vfs_mount(const uint8_t *image, const char *name)
#endif
{
	struct vfs_fs *fs = find_filesystem(name);
	VFS_ASSERT(fs || fs->mount);
	return fs->mount(image, &__vfsroot);
}

static int __vfs_lookup(struct vfs_node *dir, const char *stp, size_t namelen)
{
	char name[512];
	struct vfs_node namei;

	if (namelen > 0) {
		memcpy(name, stp, namelen);
		name[namelen] = 0;
		if (vfs_lookup(dir, name, &namei))
			return -1;
		/* printk("dir->priv=0x%x.\n", dir->priv); */
		//kfree(dir->priv);
		*dir = namei;
	}

	return 0;
}

static int fp_ucount_dec( struct vfs_node *fp )
{
	int error = 0;

	//if( (--fp->f_count) <= 0 )
	//{        
	//down(&fdlock);
        
	error = fp->vops->close(fp);

	//up(&fdlock);
            
	//}

	return error;
}

static int fd_close( int fd )
{
	int error = 0;
	struct vfs_node *fp;

	if ((fd < 0) || (fd > FD_MAX_NUM))
		return -1;

	fp = desc[fd];
	desc[fd] = FD_ALLOCATED;

	if( fp != FD_ALLOCATED && fp != NULL)
	{
		error = fp_ucount_dec( fp );
	}

	return error;
}

int fd_alloc(int low)
{
	int fd;

	if ((low < 0) || (low > FD_MAX_NUM))
		return -1;

	//down(&fdlock);
	for (fd =low; fd < FD_MAX_NUM; fd++)
	{
		if (NULL == desc[fd]) {
			desc[fd] = FD_ALLOCATED;
			//up(&fdlock);
			return fd;
		}
	}
	//up(&fdlock);
	return -1;
}

int fd_free(int fd)
{
	int error = 0;

	if ((fd < 0) || (fd > FD_MAX_NUM))
		return -1;

	//down(&fdlock);

	error = fd_close( fd );
	desc[fd] = NULL;

	//up(&fdlock);
	
	return error;
}

void fd_assign (int fd, struct vfs_node *fp)
{
	if ((fd < 0) || (fd > FD_MAX_NUM))
		return ;

	//down(&fdlock);

	fd_close( fd );

	//fp->f_count++;
	desc[fd] = fp;

	//up(&fdlock);
}

struct vfs_node *fp_get( int fd )
{
	if ((fd < 0) || (fd > FD_MAX_NUM))
		return NULL;

	//down(&fdlock);
    
	struct vfs_node *fp = desc[fd];

	if( fp != FD_ALLOCATED && fp != NULL)
	{
		//fp->f_count++;
	}
	else fp = NULL;
    
	//up(&fdlock);

	return fp;
}

void fp_free( struct vfs_node *fp )
{
	//down(&fdlock);

	fp_ucount_dec( fp );
    
	//up(&fdlock);    
}

inline struct vfs_node *file_alloc()
{
	VFS_MALLOC(struct vfs_node, fp);
	return fp;
}

int vfs_open(const char *path, struct vfs_node *file)
{
	struct vfs_opvector *vops;
	const char *p, *stp = path;
	struct vfs_node dir = __vfsroot;
	int fd;

	VFS_ASSERT(file);

	fd = fd_alloc(0);
	if (fd < 0) {
		return -1;
	}

	if (*(stp+1) != '\0') {
		for (p = stp; *p; p++) {
			if (*p == '/') {
				if (__vfs_lookup(&dir, stp, p - stp))
					return -1;
				stp = p + 1;
			}
		}

		if (__vfs_lookup(&dir, stp, p - stp))
			return -1;
	}
	vops = dir.vops;
	*file = dir;
	if (!vops->open(dir.priv)) {
		fd_assign(fd, file);
	}
	else {
		fd_free(fd);
		fd = -1;
	}
	
	return fd;
}

int vfs_read(int fd, void *buf, size_t count, size_t *ready)
{
	struct vfs_node *file = fp_get(fd);
	struct vfs_opvector *vops = file->vops;
	VFS_ASSERT(vops && vops->read);
	return vops->read(file->priv, buf, count, ready);
}

int vfs_close(int fd)
{
	struct vfs_node *file = fp_get(fd);
	struct vfs_opvector *vops = file->vops;
	VFS_ASSERT(vops && vops->close);
	fd_free(fd);
	return vops->close(file->priv);
}

int vfs_lookup(struct vfs_node *dir, const char *name, struct vfs_node *namei)
{
	VFS_ASSERT(dir && namei && name);
	struct vfs_opvector *vops = dir->vops;
	return vops->lookup(dir->priv, name, namei);
}

char *vfs_get_cur_path(void)
{
	return (char *)cur_path;
}

void vfs_change_path(char *cur_path, char *input_path)
{
	unsigned int	 len;
	char		*this, *next;
	
	if ((NULL == cur_path) || (NULL == input_path))
		return;

	while(input_path) {
		if ('/' == *input_path) {
			/* input a absolute path */
			strcpy(cur_path, input_path);
			break;
		}
		else if ((1 == strlen(input_path)) && ('.' == *input_path)) {
			break;
		}
		else if (('.' == *input_path) && ('.' != *(input_path+1))) {
			/* the input path is "./xxx" */
			if ('/' == *(cur_path + strlen(cur_path) - 1))
				strcat(cur_path, input_path+strlen("./"));
			else {
				if ((len = strlen(".")) < strlen(input_path))
					strcat(cur_path, input_path+len);
			}
			break;
		}
		else if(('.' == *input_path) && ('.' == *(input_path+1))) {
			if (strlen("/") < strlen(cur_path)) {
				/* to find the last '/' in cur_path */
				this = cur_path;
				do {
					next = this + 1;
					this = strchr(next, '/');
				} while(this);
		
				if (cur_path == (next - 1))
					*next = '\0';
				else
					*(next -1) = '\0';

				if ((len = strlen("../")) < strlen(input_path)) {
					input_path += len;
				}
				else {
					break;
				}
			}
			else {
				break;
			}
		}
		else {
			/* to add a '/' character if the last is not '/' */
			if ('/' != *(cur_path + strlen(cur_path) - 1))
				strcat(cur_path, "/");
			strcat(cur_path, input_path);
			break;
		}
	}
}
