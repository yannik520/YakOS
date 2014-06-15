CC = gcc
OBJECTS = vfs_test.o ../fs/vfsfat.o ../fs/vfsfs.o
CFLAGS := -O2 -g -I../include -D VFS_TEST

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o vfs_test

clean:
	@rm -f *.o $(OBJECTS) vfs_test
