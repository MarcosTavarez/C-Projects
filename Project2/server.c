#include "storage.h"
#include "comm.h"
#include "storage_common.h"
#include <string.h>
// By Marcos Tavarez

#define BUFSIZE 200

int main(int argc, char** argv)
{
  unsigned char buffer[BUFSIZE];
  unsigned char name[BUFSIZE];
  HEADER header;// Reading
  HEADER header_out;// Writing
  STORAGE *storage; 
  int fd_out;
  int fd_in;
  int byte_get;
  int byte_put;

 //  Loop forever (break out with SHUTDOWN)
   while(1) 
{
	fprintf(stderr, "\n");
    	fprintf(stderr, "Waiting for connection with client...\n");
	   //  Open to_storage pipe

	fd_in = open(PIPE_NAME_TO_STORAGE, O_RDONLY);// Open both pipes
	if(fd_in == -1)
	{
		fprintf(stderr,"Pipe not opened : fd_in ");
		exit(-1);
	}
	fd_out = open(PIPE_NAME_FROM_STORAGE, O_WRONLY);
	if(fd_out == -1)
	{
		fprintf(stderr,"Pipe not opened: fd_out ");
		exit(-1);
	}

	if(read(fd_in,&header,sizeof(header))!= sizeof(header)) // Read in the header
	{
		fprintf(stderr," Cannot read Header");
		exit(-1);
	}
	
		if(header.type != INIT_CONNECTION)
		{
			fprintf(stderr,"Init Connection not found\n");
			exit(-1);
		}
		fprintf(stderr,"Connection established with client\n");

			header_out.type = ACKNOWLEDGE; // Acknowledge that INIT_CONNECTION has worked
			header_out.len_message = 0;
			header_out.location = 0;

			if(read(fd_in, name, BUFSIZE) < 0)
			{
					fprintf(stderr, "Read Error: File Not found");
					exit(-1);
			}

			storage = init_storage(name); // Pass in name of file

			fprintf(stderr, "Storage file: #%s#\n", name); // Print name of file

			fprintf(stderr,"Sending Acknowledge\n");
			if(write(fd_out, &header_out, sizeof(header_out))!= sizeof(header_out)) // Write the header
			{
					fprintf(stderr," Cannot write Header that contains ACK");
					exit(-1);
			}

		while(1)
		{
			
				if(read(fd_in, &header, sizeof(header))!= sizeof(header)) // Should read the header
				{
					fprintf(stderr,"Cannot read header");
					exit(-1);
				}


				if(header.type == WRITE_REQUEST)
				{
					if(read(fd_in, buffer, BUFSIZE)!= header.len_buffer) // Should read the buffer
					{
						fprintf(stderr,"Cannot read in buffer in write_request");
						exit(-1);
					}

					byte_put = put_bytes(storage, buffer, header.location, header.len_buffer);

					if(byte_put == -1)
					{
						fprintf(stderr, "Put_byes error\n");
						exit(-1);
					}
					fprintf(stderr,"Byte_put: %d\n", byte_put);
					
					header_out.type = ACKNOWLEDGE;
					header_out.len_message = byte_put;
					header_out.location = -1;
					header_out.len_buffer = -1;

					fprintf(stderr, "Writing into file...\n");

					if(write(fd_out, &header_out, sizeof(header_out))!= sizeof(header_out)) // Send message back
					{
						fprintf(stderr,"Cannot write out header in write_request");
						exit(-1);
					}

					fprintf(stderr, "Location %d\n", header.location);
					fprintf(stderr, "Buffer length:  %d\n", header.len_buffer);

				}	
				if(header.type == READ_REQUEST)
				{
					fprintf(stderr,"header.location: %d\n", header.location);
					fprintf(stderr, "header.len_buffer: %d\n", header.len_buffer);
						
					byte_get = get_bytes(storage, buffer, header.location, header.len_buffer);

					if(byte_get == -1)
					{
						perror("Get_bytes is not returning bytes\n");
					}
					fprintf(stderr,"Byte_get: %d\n", byte_get);

					header_out.type = DATA;
					header_out.len_message = byte_get;
					header_out.location = -1;
					header_out.len_buffer = -1;

					fprintf(stderr, "Reading into file...\n");

					if(write(fd_out, &header_out, sizeof(header_out))!= sizeof(header_out)) // Send message back
					{
						fprintf(stderr,"Cannot write out header in read_request");
						exit(-1);
					}

					if(write(fd_out, buffer, header_out.len_message)!= header_out.len_message) // Write the buffer back into the remote
					{
						fprintf(stderr,"Cannot write out buffer in read_request");
						exit(-1);
					}
				}	
				if(header.type == SHUTDOWN)
				{
					fprintf(stderr ,"SHUTING DOWN...\n");
					header_out.type = ACKNOWLEDGE;
					if(write(fd_out, &header_out, sizeof(header_out))!= sizeof(header_out))
					{
						fprintf(stderr,"Cannot write out Ack in shutdown");
						exit(-1);
					}

					sleep(1);	
					break;
				}
		}
	
    // We broke out because of a disconnection: clean up
    	fprintf(stderr, "Closing connection\n");
    	close(fd_in);
    	close(fd_out);
   	close_storage(storage);
	
}

  // Should never reach here
fprintf(stderr,"Should not be here");
  return(0);
}
