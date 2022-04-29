/**
   API for remote storage.  Identical API as provided by the local storage implementation 
 */
#include <string.h>
#include "storage_remote.h"
#include "storage_common.h"
#include "comm.h"

/**
   initialize the storage

   Open the two fds (in and out), wait for an init message, initialize the local storage
 */
STORAGE * init_storage(char * name)
{
  // Create space for the STORAGE object
	HEADER h; // Header struct
	h.len_message = 0;
	h.location = -1;
	h.len_buffer = -1;
	// Initialize the HEADER
	h.type = INIT_CONNECTION; // Initialize type
	STORAGE *s = malloc(sizeof(STORAGE)); // malloc enough memory in heap

	s->fd_to_storage = open(PIPE_NAME_TO_STORAGE, O_WRONLY); // Open pipe for writing
	s->fd_from_storage = open(PIPE_NAME_FROM_STORAGE, O_RDONLY); // Open pipe for readin

	if(write(s->fd_to_storage,&h,sizeof(h))!= sizeof(h)) // Send init message
	{
		fprintf(stderr,"Cannot write header in init");
		exit(-1);
	}

	write(s->fd_to_storage, name, strlen(name) + 1);
	// Init message sent and Name of file sent


	if(read(s->fd_from_storage, &h, sizeof(h)) < 0)
	{
		fprintf(stderr,"Read error in init_storage");
		exit(-1);
	}
	if(h.type == ACKNOWLEDGE)
	{
		fprintf(stderr,"ACK RECIEVED\n");
	}

	//Must recieve acknowledge


  // All okay 
  return (s);
}

/**
   Shut down the connection

   Tell the server to shut down
 */
int close_storage(STORAGE *storage)
{
  // Create the shutdown message
  	HEADER header;
	header.len_message = 0;
	header.location = -1;
	header.len_buffer = -1;
	// Initialize HEADER
	header.type = SHUTDOWN;
	if(write(storage->fd_to_storage, &header, sizeof(header))!= sizeof(header))
	{
		fprintf(stderr,"Error in close header");
		exit(-1);
	}
	
	if(read(storage->fd_from_storage, &header, sizeof(header)) < 0)
	{
		fprintf(stderr,"Read error in close_storage\n");
		exit(-1);
	}
	else if(header.type == ACKNOWLEDGE)
	{
		fprintf(stderr,"SHUT DOWN ACKNOWLEDGED\n");
		return(0);
	}
  // Free the storage struction
  free(storage);
  // Done
  return(0);
}

/**
   read bytes from the storage
 */
int get_bytes(STORAGE *storage, unsigned char *buf, int location, int len)
{
	HEADER h;
	h.len_message = 0;
	h.type = READ_REQUEST;
	h.location = location;
	h.len_buffer = len;
	
	if(write(storage->fd_to_storage, &h, sizeof(h))!= sizeof(h)) // Write in the header
	{
		fprintf(stderr,"Cannot write to storage in get bytes");
		exit(-1);
	}
	
	if(read(storage->fd_from_storage, &h, sizeof(h)) < 0)
	{
		fprintf(stderr, "Error in get_bytes\n");
		exit(-1);
	}
	if(h.type != DATA )
	{
		fprintf(stderr,"Data Not Read\n");
		exit(-1);
	}
	fprintf(stderr,"Data Read\n");
	if(read(storage->fd_from_storage, buf, h.len_message) < 0)
	{
		fprintf(stderr,"Cannot read back buffer in get bytes");
		exit(-1);
	}
//Write Request? Not actuall reading or writing anything, as in the buffer, only sending the information to the server
// Data request?

  // Success
  return(h.len_message);
}


/**
   Write bytes to the storae

   Send the write request message + the data
 */
int put_bytes(STORAGE *storage, unsigned char *buf, int location, int len)
{
	HEADER h;
	h.len_message = 0;
	h.type = WRITE_REQUEST;
	h.location = location;
	h.len_buffer = len;

	 if(write(storage->fd_to_storage,&h,sizeof(h))!= sizeof(h))
	{
		fprintf(stderr,"Error in writing to storage in put bytes");
		exit(-1);
	}

	 if(write(storage->fd_to_storage, buf, len)!= len)
	{
		fprintf(stderr, "Error in writing to storage in put bytes");
		exit(-1);
	}

	if(read(storage->fd_from_storage, &h, sizeof(h)) < 0)
	{
		fprintf(stderr, "Error in put_bytes\n");
		exit(-1);
	}
	else if(h.type == ACKNOWLEDGE)
	{
		fprintf(stderr, "Write Acknowledged\n");
	}
//Read request?
//Data Request?

  // Success

  return(len);
}

