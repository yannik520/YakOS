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

#include <stdio.h>
#include <fs/vfsfs.h>
#include <fs/vfsfat.h>

int main(int argc, char *argv[])
{
	size_t count;
	char buf[1024];
	struct vfs_node file;
	int fd;
	char new_path[128] = "/";

	if (argc < 2) {
		printf("too few argument!\n");
		return -1;
	}

	register_filesystem(&fat_fs);

	if (vfs_mount(argv[1], "FAT")) {
		printf("vfs_mount fail\n");
		return -1;
	}

	for (int i=0; i<2; i++) {
		if ((fd = vfs_open(argv[2], &file)) == -1) {
			printf("vfs_open fail\n");
			return -1;
		}
		printf("fd=%d\n", fd);

		FILE * fout = fopen(argc < 4? "fout.txt": argv[3], "wb");
		if (fout != NULL) {
			while (!vfs_read(fd, buf, sizeof(buf), &count) && count > 0) {
				printf("%s\n", buf);
				fwrite(buf, 1, count, fout);
			}
			fclose(fout);
		}

		if (vfs_close(fd)) {
			printf("vfs_close fail\n");
			return -1;
		}

	}

	printf("Test vfs_change_path:\n");
	printf("test 1: /# ls /\n");
	vfs_change_path(new_path, "/");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/");

	printf("test 2: /# ls .\n");
	vfs_change_path(new_path, ".");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/");

	printf("test 3: /# ls ..\n");
	vfs_change_path(new_path, "..");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/");

	printf("test 4: /# ls ../..\n");
	vfs_change_path(new_path, "../..");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/");

	printf("test 5: /# ls ../abc\n");
	vfs_change_path(new_path, "../abc");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/");

	printf("test 6: /# ls ./abc\n");
	vfs_change_path(new_path, "./abc");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/");

	printf("test 7: /# ls ./abc/def\n");
	vfs_change_path(new_path, "./abc/def");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx");

	printf("test 8: /xxx# ls /\n");
	vfs_change_path(new_path, "/");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx");

	printf("test 9: /xxx# ls .\n");
	vfs_change_path(new_path, ".");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx");

	printf("test 10: /xxx# ls ..\n");
	vfs_change_path(new_path, "..");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx");

	printf("test 11: /xxx# ls ../..\n");
	vfs_change_path(new_path, "../..");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx");

	printf("test 12: /xxx# ls ../abc\n");
	vfs_change_path(new_path, "../abc");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx");

	printf("test 13: /xxx# ls ./abc\n");
	vfs_change_path(new_path, "./abc");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx");

	printf("test 14: /xxx# ls ./abc/def\n");
	vfs_change_path(new_path, "./abc/def");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx");

	printf("test 15: /xxx/xxx# ls /\n");
	vfs_change_path(new_path, "/");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx/xxx");

	printf("test 16: /xxx/xxx# ls .\n");
	vfs_change_path(new_path, ".");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx/xxx");

	printf("test 17: /xxx/xxx# ls ..\n");
	vfs_change_path(new_path, "..");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx/xxx");

	printf("test 18: /xxx/xxx# ls ../..\n");
	vfs_change_path(new_path, "../..");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx/xxx");

	printf("test 19: /xxx/xxx# ls ../abc\n");
	vfs_change_path(new_path, "../abc");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx/xxx");

	printf("test 20: /xxx/xxx# ls ./abc\n");
	vfs_change_path(new_path, "./abc");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx/xxx");

	printf("test 21: /xxx/xxx# ls ./abc/def\n");
	vfs_change_path(new_path, "./abc/def");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx/xxx/xxx");

	printf("test 22: /xxx/xxx/xxx# ls ../..\n");
	vfs_change_path(new_path, "../..");
	printf("-> %s\n\n", new_path);
	strcpy(new_path, "/xxx/xxx/xxx");

	printf("test success\n");
	return 0;
}

