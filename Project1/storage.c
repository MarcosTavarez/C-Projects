#include"storage.h"
//#include<sys/types.h>
//#include<sys/stat.h>
//#include<fcntl.h>

///////////////////////////////////////////////////////////////
STORAGE* init_storage(char*name)
{
printf("Made it into init_storage\n");
	STORAGE* S = malloc(sizeof(*S));// Storage struct
	S->fd = open( "storage.bin" , O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // Open/Create a file
	if( S->fd == -1)
		{
			perror("Unable to open file");
			exit(-1);	
		}
	return(S);
//Open is succesful yay
}
///////////////////////////////////////////////////////////////
int put_bytes(STORAGE* S, unsigned char* buf, int location, int len)
{
	int ret;
	ret = lseek(S->fd, location,SEEK_SET);	
	if(ret == -1)
	{
		perror("Seek Error");
	}
	//Done with seek
	if(write(S->fd, buf, len) != len )
	{
		perror("Write error");
		exit(-1);
	}
//Writing into file
	
	return(len);
}

///////////////////////////////////////////////////////////////
int get_bytes(STORAGE* S, unsigned char *buf, int location, int len)
{
	int ret;
	//int read_int;
	ret = lseek(S->fd, location,SEEK_SET);
	if(ret == -1)
	{
		perror("Seek Error");
	}
	if(read(S->fd, buf, len) < 0 )
	{
		perror("Read error");
		exit(-1);
	}
	else
	{
		perror("EOF");
	}

	return(len);
//Reading into file
}

///////////////////////////////////////////////////////////////
int close_storage(STORAGE* S)
{
	int num;
	num = close(S->fd);
	free(S);
	return(num);
}
//Closing file
