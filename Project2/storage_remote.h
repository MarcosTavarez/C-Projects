#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct 
{
  int fd_to_storage;
  int fd_from_storage;
} STORAGE;


STORAGE * init_storage(char * name);
int close_storage(STORAGE *storage);
int get_bytes(STORAGE *storage, unsigned char *buf, int location, int len);
int put_bytes(STORAGE *storage, unsigned char *buf, int location, int len);

