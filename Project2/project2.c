#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#define BUFF_SIZE 128 //Define the buffer size
#include "storage_remote.h"
//Marcos Version 4


const char SEPERATORS[] = " \t\n"; //Global constant

void zeroOut(unsigned char *buffer)//Zero out the contents in our Buffer
{
	memset(buffer,0,BUFF_SIZE);
}

void printBuff(unsigned char *buffer) // Print out contents in our buffer
{
		int i = 0;
		

		for(int k = 0; k < 8; k++)
		{
			printf("\n"); // Newline
			for(int l = 0; l < 16; l++)
			{
				printf("%02x ", buffer[i]);
				i++;
			}
		}
	printf("\n");
}

void write_b(int location, int value, unsigned char *buffer) // Write byte into Buffer
{
	memcpy(buffer + location,&value, 1);
	
}

void read_B(int location, unsigned char *buffer)// Read byte from buffer
{
	int a;
	memcpy(&a, buffer + location, 1);
	
	printf("%d\n", a); // Given a location, a value will be printed out
}

void write_h(int location, unsigned char hex, unsigned char *buffer)//Write a hexadecimal byte value to a location
{
	memcpy(buffer + location, &hex, sizeof(unsigned char));
}

void read_H(int location, unsigned char *buffer)
{
	printf("%x\n", buffer[location]);
}

void write_c(int location, char *value_char, unsigned char *buffer) // Write a character to a location
{
	strcpy(buffer + location, value_char);
}

void read_C(int location, unsigned char *buffer)
{
	char c_char;
	memcpy(&c_char,buffer + location, sizeof(unsigned char));
	printf("%c\n",c_char);
}

void write_i(int location, int value, unsigned char* buffer) // integer value(In Decimal)
{
	memcpy(buffer + location ,&value, sizeof(int));
}

void read_I(int location, unsigned char *buffer)
{
	int a;

	memcpy(&a, buffer + location, sizeof(int));	

	printf("%i\n", a);
					
}

void write_f(int location, float value, unsigned char *buffer)// Write a float value to a location
{
	float *ptr = &value;
	memcpy(buffer + location, ptr, sizeof(float));
}

void read_F(int location, unsigned char *buffer)// Print at location
{
	float a;
	memcpy(&a, buffer + location, sizeof(float));
	printf("%f\n", a);
}
void write_s(int location, char *string,unsigned char* buffer )// Write a string to a location
{
	strcpy(buffer + location, string); 
}

void read_S(int location, unsigned char *buffer)// Read String from buffer
{
	char string[128];
	strcpy( string,buffer + location);
	printf("%s\n", string); 
}

void write_w(int offSet, int length_bytes)// Offset is the offset in the file to start writing to, length_bytes is the number of bytes in the buffer to write
{
	unsigned char input_buffer[BUFF_SIZE]; //Input buffer set as a global variable
	STORAGE* store =  init_storage("storage.bin");
	if(put_bytes(store, input_buffer,offSet,length_bytes)== -1)
	{
		perror("Write Failed");
		exit(-1);
	}
	else
	{
		int close;
		close = close_storage(store);
		if(close != 0)
		{
			perror("Could not close file");
			exit(-1);
		}
	}
	
}

void read_r(int offSet, int length_bytes)// Offset is the offset in the file to start reading from. Length_bytes is the number of bytes to read
{
	unsigned char input_buffer[BUFF_SIZE]; //Input buffer set as a global variable
	STORAGE* store = init_storage("storage.bin");
	if(get_bytes(store,input_buffer,offSet,length_bytes) == -1)
	{
		perror("Read failed");
		exit(-1);
	}
	else
	{
		int close;
		close = close_storage(store);
		if(close != 0)
		{
			perror("Could not close file");
			exit(-1);
		}
	}

}
int main(int argc, char **argv)
{
	unsigned char input_buffer[BUFF_SIZE]; //Input buffer set as a global variable
	char command_buffer[1000];
	char *args[1000];
	char **arg;
	STORAGE* stor;
	zeroOut(input_buffer);
	if(argv[1] != NULL)
	{
	stor = init_storage(argv[1]);
	}
	else
	{
	 stor = init_storage("storage.bin");
	}
	
	while(fgets(command_buffer,128,stdin))// Takes in commands
	{
		arg = args;
		*arg++ = strtok(command_buffer,SEPERATORS);
		while((*arg++ = strtok(NULL,SEPERATORS)));
			if(*args[0] == 'z')// Zeros out the bytes
			{
				zeroOut(input_buffer);
			}
			else if(*args[0] == 'l')// Prints out the bytes in a HexDump
			{
				printBuff(input_buffer);
			}
			else if(*args[0] == 'b')// Write byte value to buffer LOCATION, VALUE is a decimal
			{
				write_b(atoi(args[1]) , atoi(args[2]), input_buffer); 
			}
			else if(*args[0] == 'B')// Read a byte value from a LOCATION and print it in decimal
			{
				read_B(atoi(args[1]), input_buffer);
			}
			else if(*args[0] == 'h')// Read a byte value from a LOCATION, VALUE iS in HEXADECIMAL
			{
				unsigned int a;
				sscanf(args[2],"%x",&a);
				write_h(atoi(args[1]),(unsigned char)a, input_buffer);

			}
			else if(*args[0] == 'H')// Read a byte value from a LOCATION and print it in HEXADECIMAL
			{
				read_H(atoi(args[1]),input_buffer);
			}
			else if(*args[0] == 'c')
			{
				char a[50];
				sscanf(args[2],"%s",a);
				write_c(atoi(args[1]),a, input_buffer);
			}
			else if(*args[0] == 'C')
			{
				read_C(atoi(args[1]), input_buffer);
			}
			else if(*args[0] == 'i')// Read a byte value from a LOCATION, VALUE is in decimal
			{
				write_i(atoi(args[1]),atoi(args[2]), input_buffer);
			}
			else if(*args[0] == 'I')// Read a byte value from a LOCATION and print it in decimal
			{
				read_I(atoi(args[1]), input_buffer);
			}
			else if(*args[0] == 'f')// Read a byte value from a LOCATION, VALUE is in floating point
			{
				float val =(float)atof(args[2]);
				write_f(atoi(args[1]),val, input_buffer);
			}
			else if(*args[0] == 'F')// Read a byte value from a LOCATION and print it in floating point
			{
				read_F(atoi(args[1]), input_buffer);
			}
			else if(*args[0] == 's')// Read a byte value from a LOCATION, VALUE is in string value
			{
				char string[128];
				sscanf(args[2],"%s", string);
				write_s(atoi(args[1]), string, input_buffer); // TODO
			}
			else if(*args[0] == 'S')// Read a byte value from a LOCATION and print it in string value
			{
				read_S(atoi(args[1]), input_buffer);
			}
			else if(*args[0] == 'w')// Write into the file from the buffer
			{
				int offset = atoi(args[1]);
				int length_bytes = atoi(args[2]);

				if(put_bytes(stor,input_buffer,offset,length_bytes) == -1)
				{
					perror("Write Failed");
					exit(-1);
				}
			//	write_w(atoi(args[1]), atoi(args[2]));
			}	
			else if(*args[0] == 'r')// Read from the file into the buffer
			{
				int off = atoi(args[1]);
				int length = atoi(args[2]);

				if(get_bytes(stor, input_buffer,off,length) == -1)
				{
					perror("Read Failed");
					exit(-1);
				}
			//	read_r(atoi(args[1]), atoi(args[2]));
			}
			
			else
			{
				fprintf(stderr ,"Incorrect Commands entered\n");
				exit(-1);
			}
	}	

			if(close_storage(stor) < 0)
			{
				fprintf(stderr,"Close error\n");
				exit(-1);
			}
}
