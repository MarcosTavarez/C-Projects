:#include<sys/types.h>
#include<sys/stat.h>

LIBS = storage.o virtual_disk.o oufs_lib.o oufs_lib_support.o
CFLAGS = -g -Wall -c
CC = gcc
EXECS = oufs_format oufs_inspect oufs_ls oufs_mkdir oufs_rmdir oufs_stats
INCLUDES = oufs.h oufs_lib_support.h virtual_disk.h oufs_lib.h storage.h

all: $(EXECS)

oufs_format: oufs_format.o $(LIBS) $(INCLUDES)
	$(CC) $< $(LIBS) -o $@

oufs_inspect: oufs_inspect.o $(LIBS) $(INCLUDES)
	$(CC) $< $(LIBS) -o $@

oufs_ls: oufs_ls.o $(LIBS) $(INCLUDES)
	$(CC) $< $(LIBS) -o $@

oufs_mkdir: oufs_mkdir.o $(LIBS) $(INCLUDES)
	$(CC) $< $(LIBS) -o $@

oufs_rmdir: oufs_rmdir.o $(LIBS) $(INCLUDES)
	$(CC) $< $(LIBS) -o $@

oufs_stats: oufs_stats.o $(LIBS) $(INCLUDES)
	$(CC) $< $(LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECS)
