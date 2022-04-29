
#include "oufs_lib.h"
#include "oufs_lib_support.h"
#include "virtual_disk.h"
#include <math.h>
#include <stdio.h>
void printBuff(unsigned char *buffer)
{
	int i = 0;
	for(int k = 0; k < 8; k++)
	{
		fprintf(stderr,"\n");
		for(int l = 0; l < 16; l++)
		{
			fprintf(stderr, "%02x ", buffer[i]);
			i++;
		}
	}
fprintf(stderr,"\n");
}

// Yes ... a global variable
int debug = 1;

// Translate inode types to descriptive strings
const char *INODE_TYPE_NAME[] = { "UNUSED", "DIRECTORY", "FILE" };

/**
 Read the OUFS_PWD, OUFS_DISK, OUFS_PIPE_NAME_BASE environment
 variables copy their values into cwd, disk_name an pipe_name_base.  If these
 environment variables are not set, then reasonable defaults are
 given.

 @param cwd String :buffer in which to place the OUFS current working directory.
 @param disk_name String buffer in which to place file name of the virtual disk.
 @param pipe_name_base String buffer in which to place the base name of the
			named pipes for communication to the server.

 PROVIDED
 */
void oufs_get_environment(char *cwd, char *disk_name, char *pipe_name_base)
{
	// Current working directory for the OUFS
	char *str = getenv("OUFS_PWD");
	if (str == NULL)
	{
		// Provide default
		strcpy(cwd, "/");
	}
	else
	{
		// Exists
		strncpy(cwd, str, MAX_PATH_LENGTH - 1);
	}

	// Virtual disk location
	str = getenv("OUFS_DISK");

	if (str == NULL)
	{
		// Default
		strcpy(disk_name, "vdisk1");
	}
	else
	{
		// Exists: copy
		strncpy(disk_name, str, MAX_PATH_LENGTH - 1);
	}

	// Pipe name base
	str = getenv("OUFS_PIPE_NAME_BASE");

	if (str == NULL)
	{
		// Default
		strcpy(pipe_name_base, "pipe");
	}
	else
	{
		// Exists: copy
		strncpy(pipe_name_base, str, MAX_PATH_LENGTH - 1);
	}

}

/**
 * Completely format the virtual disk (including creation of the space).
 *
 * NOTE: this function attaches to the virtual disk at the beginning and
 *  detaches after the format is complete.
 *
 * - Zero out all blocks on the disk.
 * - Initialize the master block: mark inode 0 as allocated and initialize
 *    the linked list of free blocks
 * - Initialize root directory inode
 * - Initialize the root directory in block ROOT_DIRECTORY_BLOCK
 *
 * @return 0 if no errors
 *         -x if an error has occurred.
 *
 */

int oufs_format_disk(char  *virtual_disk_name, char *pipe_name_base)
{
	// Attach to the virtual disk
	if (virtual_disk_attach(virtual_disk_name, pipe_name_base) != 0)
	{
		return(-1);
	}

	BLOCK block;

	// Zero out the block
	memset(&block, 0, BLOCK_SIZE);

	for (int i = 0; i < N_BLOCKS; ++i)
	{
		if (virtual_disk_write_block(i, &block) < 0)
		{
			return(-2);
		}
	}

	//Read block(Marcos)
	//Change block(Marcos)
	//Write block (Marcos)

	  //////////////////////////////
	  // Master block
	block.next_block = UNALLOCATED_BLOCK;
	block.content.master.inode_allocated_flag[0] = 0x80;

	block.content.master.unallocated_front = 6; // Allocate front of master block to next block(Marcos)

	block.content.master.unallocated_end = N_BLOCKS - 1; // Allocate end block to block 127(Marcos)


	virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &block);


	// TODO:  complete implementation

	//////////////////////////////
	// Root directory inode / block
	INODE inode;
	oufs_init_directory_structures(&inode, &block, ROOT_DIRECTORY_BLOCK, ROOT_DIRECTORY_INODE, ROOT_DIRECTORY_INODE);

	// Write the results to the disk
	if (oufs_write_inode_by_reference(0, &inode) != 0)
	{
		return(-3);
	}

	virtual_disk_write_block(ROOT_DIRECTORY_BLOCK, &block); // Write the root block

	  // TODO: complete implementation


	memset(&block, 0, BLOCK_SIZE);// Clear out blocks again

	//////////////////////////////////////////////////////
	 // All other blocks are free blocks

	for (BLOCK_REFERENCE i = 7; i < N_BLOCKS; i++)
	{
		block.next_block = i;
		virtual_disk_write_block(i - 1, &(block)); // Write free blocks
		if (i == 127)
		{
			block.next_block = UNALLOCATED_BLOCK;
			virtual_disk_write_block(i, &block);
		}
		// Last block should point to UNALLOCATED BLOCK
	}
	// TODO: complete


	// Done
	virtual_disk_detach();

	return(0);
}

/*
 * Compare two inodes for sorting, handling the
 *  cases where the inodes are not valid
 *
 * @param e1 Pointer to a directory entry
 * @param e2 Pointer to a directory entry
 * @return -1 if e1 comes before e2 (or if e1 is the only valid one)
 * @return  0 if equal (or if both are invalid)
 * @return  1 if e1 comes after e2 (or if e2 is the only valid one)
 *
 * Note: this function is useful for qsort()
 */
static int inode_compare_to(const void *d1, const void *d2)
{
	// Type casting from generic to DIRECTORY_ENTRY*
	DIRECTORY_ENTRY* e1 = (DIRECTORY_ENTRY*)d1;
	DIRECTORY_ENTRY* e2 = (DIRECTORY_ENTRY*)d2;
	int result = strcmp(e1->name, e2->name); // assign result to what ever strcmp returns


	if ((e1->inode_reference != UNALLOCATED_INODE) && (e2->inode_reference != UNALLOCATED_INODE))
	{
		return (result);
	}
	else if ((e1->inode_reference == UNALLOCATED_INODE) && (e2->inode_reference != UNALLOCATED_INODE))
	{
		return (-1);
	}

	else if ((e1->inode_reference != UNALLOCATED_INODE) && (e2->inode_reference == UNALLOCATED_INODE))
	{
		return (1);
	}
	// TODO: complete implementation
	return(-13); // Error, comparison not made

}


/**
 * Print out the specified file (if it exists) or the contents of the
 *   specified directory (if it exists)
 *
 * If a directory is listed, then the valid contents are printed in sorted order
 *   (as defined by strcmp()), one per line.  We know that a directory entry is
 *   valid if the inode_reference is not UNALLOCATED_INODE.
 *   Hint: qsort() will do to sort for you.  You just have to provide a compareTo()
 *   function (just like in Java!)
 *   Note: if an entry is a directory itself, then its name must be followed by "/"
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Absolute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 */

int oufs_list(char *cwd, char *path)
{
	INODE_REFERENCE parent;
	INODE_REFERENCE child;

	// Look up the inodes for the parent and child
	int ret = oufs_find_file(cwd, path, &parent, &child, NULL);

	// Did we find the specified file?
	if (ret == 0 && child != UNALLOCATED_INODE)
	{
		// Element found: read the inode
		INODE inode;
		if (oufs_read_inode_by_reference(child, &inode) != 0)
		{
			return(-1);
		}
		if (debug)
		{
			fprintf(stderr, "\tDEBUG: Child found (type=%s).\n", INODE_TYPE_NAME[inode.type]);
		}

		INODE inode_2;

		BLOCK child_block;
		virtual_disk_read_block(inode.content, &child_block);
		//Read in directory block to list
		qsort(child_block.content.directory.entry, N_DIRECTORY_ENTRIES_PER_BLOCK, sizeof(DIRECTORY_ENTRY), inode_compare_to);
		// sort the entries
		//TODO Find out how to compare directories and list their contents(Marcos)
		for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++)
		{
			if(child_block.content.directory.entry[i].inode_reference != UNALLOCATED_INODE) // Inode must exist
			{
				oufs_read_inode_by_reference(child_block.content.directory.entry[i].inode_reference,&inode_2);

				if(inode_2.type == DIRECTORY_TYPE)
				{
					printf("%s/\n", child_block.content.directory.entry[i].name); // Print entry
				//	fprintf(stderr,"child_inode type : %d\n ", child_block.content.inodes.inode[i].type);
				}
				else
				{
					printf("%s\n", child_block.content.directory.entry[i].name);
				}
			}

		}

		// TODO: complete implementation

	}
	else
	{
		// Did not find the specified file/directory
		fprintf(stderr, "Not found\n");
		if (debug)
		{
			fprintf(stderr, "\tDEBUG: (%d)\n", ret);
		}
	}
	// Done: return the status from the search
	return(ret);
}




///////////////////////////////////
/**
 * Make a new directory
 *
 * To be successful:
 *  - the parent must exist and be a directory
 *  - the parent must have space for the new directory
 *  - the child must not exist
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Absolute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 */
int oufs_mkdir(char *cwd, char *path)
{
	INODE_REFERENCE parent;
	INODE_REFERENCE child;

	// Name of a directory within another directory
	char local_name[MAX_PATH_LENGTH];
	int ret;
	fprintf(stderr, "Before find fill\n");
	// Attempt to find the specified directory
	if ((ret = oufs_find_file(cwd, path, &parent, &child, local_name)) < -1)
	{
		if (debug)
		{
			fprintf(stderr, "oufs_mkdir(): ret = %d\n", ret);
		}
		return(-1);
	};

	if (child != UNALLOCATED_INODE || parent == UNALLOCATED_INODE)
	{
		fprintf(stderr, "Child is Allocated or parent is not\n");
		return(-1);
	}


	BLOCK b; // Block we just initiated
	INODE inode; // Inode we just initiated
	INODE_REFERENCE child_inode_ref;

	fprintf(stderr, "Before allocate new dir call\n");

	fprintf(stderr, "Child is: %d, Parent is: %d\n", child, parent);

	oufs_read_inode_by_reference(parent, &inode);//Read the parent inode

	virtual_disk_read_block(inode.content, &b); // Parent block

	child_inode_ref = oufs_allocate_new_directory(parent); // Get inode Reference and allocated a new directory
	//oufs_allocate_new_directory(parent); // Not sure

	for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++)
	{
		if (inode.size <= N_DIRECTORY_ENTRIES_PER_BLOCK && inode.type == DIRECTORY_TYPE)
		{
			if (b.content.directory.entry[i].inode_reference == UNALLOCATED_INODE)
			{
				strcpy(b.content.directory.entry[i].name, local_name); // Set name
				b.content.directory.entry[i].inode_reference = child_inode_ref; // Set inode_reference
				break;

			}
		}
		else
		{
			fprintf(stderr, "Not enough space or not a DIRECTORY_TYPE\n");
		}
	}

	//parent = inode_reference;

	inode.size += 1;//Update the size

	oufs_write_inode_by_reference(parent, &inode); // Write contents of inode to disk
	virtual_disk_write_block(inode.content, &b); // Write Block contents to disk
	  // TODO: complete implementation

}

/**
 * Remove a directory
 *
 * To be successul:
 *  - The directory must exist and must be empty
 *  - The directory must not be . or ..
 *  - The directory must not be /
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Abslute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 */
int oufs_rmdir(char *cwd, char *path)
{
	INODE_REFERENCE parent;
	INODE_REFERENCE child;
	char local_name[MAX_PATH_LENGTH];

	// Try to find the inode of the child
	if (oufs_find_file(cwd, path, &parent, &child, local_name) < -1)
	{
		return(-4);
	}

	if (child == UNALLOCATED_INODE)
	{
		fprintf(stderr, "Child should not be UNALLOCATED!\n");
	}


	BLOCK master_block;
	virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &master_block);

	BLOCK parent_block;
	INODE parent_inode;
	INODE child_inode;
	BLOCK child_block;

	oufs_read_inode_by_reference(parent, &parent_inode); // Read in parent inode
	oufs_read_inode_by_reference(child, &child_inode); // Read in child inode

	virtual_disk_read_block(parent_inode.content, &parent_block); // Read in child block
	virtual_disk_read_block(child_inode.content, &child_block);// Read in child block

	if (child_inode.size > 2) // Still directories in child, cannot delete entries within parent
	{
		return(-3);
	}
	if ((strcmp(local_name, ".")) == 0) // Cannot delete .
	{
		return(-8);
	}
	if ((strcmp(local_name, "..")) == 0) // Cannot delete ..
	{
		return(-3);
	}
	if (child_inode.content == UNALLOCATED_BLOCK) // Block should exist
	{
		return(-3);
	}
	int byte = child / 8; // Find byte location of INODE_REFERENCE
	int bit = 7 - (child % 8);//Find bit location within bit array

	fprintf(stderr, "Child is: %d\n ", child);
	fprintf(stderr, "Parent is: %d\n", parent);
	fprintf(stderr, "byte is: %d\n", byte);
	fprintf(stderr, "bit is: %d\n", bit);
	fprintf(stderr, "Parent size is: %d\n", parent_inode.size);
	fprintf(stderr, "Child size is: %d\n", child_inode.size);

	oufs_deallocate_block(&master_block, child_inode.content);
	for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++)
	{
		if ((strcmp(parent_block.content.directory.entry[i].name, local_name) == 0))// Look for name if it even exists
		{
			fprintf(stderr, "Everything should be removed\n");
			parent_block.content.directory.entry[i].inode_reference = UNALLOCATED_INODE; // Set that inode to unallocated_inode, effectively deallocating it
		}
	}

	parent_inode.size -= 1;	//Decrement size of newly deallocated entry within parent
	master_block.content.master.inode_allocated_flag[byte] ^= 1 << bit; // Flip bit back to 0
	oufs_write_inode_by_reference(parent, &parent_inode); // Write out parent inode
	virtual_disk_write_block(parent_inode.content, &parent_block); // Write parent block

	virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &master_block); // Write master block
	virtual_disk_write_block(child_inode.content, &child_block);
	oufs_write_inode_by_reference(child, &child_inode); // Write out child inode
// TODO: complet implementation


// Success
	return(0); // Yay
}


/*********************************************************************/
// Project 4
/**
 * Open a file
 * - mode = "r": the file must exist; offset is set to 0
 * - mode = "w": the file may or may not exist;
 *                 - if it does not exist, it is created 
 *                 - if it does exist, then the file is truncated
 *                       (size=0 and data blocks deallocated);
 *                 offset = 0 and size = 0
 * - mode = "a": the file may or may not exist
 *                 - if it does not exist, it is created 
 *                 offset = size
 *
 * @param cwd Absolute path for the current working directory
 * @param path Relative or absolute path for the file in question
 * @param mode String: one of "r", "w" or "a"
 *                 (note: only the first character matters here)
 * @return Pointer to a new OUFILE structure if success
 *         NULL if error
 */

OUFILE* oufs_fopen(char *cwd, char *path, char *mode)
{

INODE_REFERENCE parent;
INODE_REFERENCE child;
char local_name[MAX_PATH_LENGTH];
INODE inode;
int ret;

  	// Check for valid mode
if(mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a') 
{	
    	fprintf(stderr, "fopen(): bad mode.\n");
    	return(NULL);
};

  // Try to find the inode of the child
if((ret = oufs_find_file(cwd, path, &parent, &child, local_name)) < -1) 
{
    if(debug)
	{
    		fprintf(stderr, "oufs_fopen(%d)\n", ret);
    		return(NULL);
	}
}
  
if(parent == UNALLOCATED_INODE) 
{
	fprintf(stderr, "Parent directory not found.\n");
    	return(NULL);
}


OUFILE *ou_file = malloc(sizeof(*ou_file)); // Remember to malloc

fprintf(stderr,"Child inode reference is: %d\n", child);

if(mode[0] == 'r')
{
	if(child == UNALLOCATED_INODE)
	{
		fprintf(stderr,"Could not read file\n");
		return(NULL);
	}
	oufs_read_inode_by_reference(child,&inode);
	if(inode.type == DIRECTORY_TYPE)
	{
		return(NULL);
	}
	ou_file->inode_reference = child;
	ou_file->mode = 'r';
	ou_file->offset = 0;

}	
if(mode[0] == 'w')
{
	fprintf(stderr,"Made it into 'w'\n");	
	ou_file->mode = 'w';
	if(child == UNALLOCATED_INODE)//File does not exist
	{
	
		fprintf(stderr,"File did not exist, creating file\n");
		ou_file->inode_reference = oufs_create_file(parent,local_name);
		
	}
	else // File does exist
	{
		oufs_read_inode_by_reference(child,&inode);
		if(inode.type == DIRECTORY_TYPE)
		{
			return(NULL);
		}
		else
		{
		ou_file->inode_reference = child;
		inode.size = 0;
		ou_file->offset = 0;
		if(oufs_deallocate_blocks(&inode) < 0)
		{
			return(NULL);
		}
		inode.content = UNALLOCATED_BLOCK;
		fprintf(stderr,"File exists, Deallocating block\n");
		oufs_write_inode_by_reference(child,&inode);
		}
	}
}
if(mode[0] == 'a')
{
	ou_file->mode = 'a';
	if(child == UNALLOCATED_INODE) // File does not exist
	{
		fprintf(stderr,"Made it into 'a'\n");	
		ou_file->inode_reference = oufs_create_file(parent,local_name);
		
	}
	else // File exists
	{
		oufs_read_inode_by_reference(child,&inode);
		if(inode.type == DIRECTORY_TYPE)
		{
			return(NULL);
		}
		ou_file->inode_reference = child;
		ou_file->offset = inode.size;
		oufs_write_inode_by_reference(child,&inode);
	}
}

  // TODO

  return(ou_file);
};

/**
 *  Close a file
 *   Deallocates the OUFILE structure
 *
 * @param fp Pointer to the OUFILE structure
 */
     
void oufs_fclose(OUFILE *fp) 
{
  fp->inode_reference = UNALLOCATED_INODE;
  free(fp);
}



/*
 * Write bytes to an open file.
 * - Allocate new data blocks, as necessary
 * - Can allocate up to MAX_BLOCKS_IN_FILE, at which point, no more bytes may be written
 * - file offset will always match file size; both will be updated as bytes are written
 *
 * @param fp OUFILE pointer (must be opened for w or a)
 * @param buf Character buffer of bytes to write
 * @param len Number of bytes to write
 * @return The number of written bytes
 *          0 if file is full and no more bytes can be written
 *         -x if an error
 * 
 */
int oufs_fwrite(OUFILE *fp, unsigned char * buf, int len)
{

  INODE inode;
  BLOCK block;

if(fp->mode == 'r') 
{
    fprintf(stderr, "Can't write to read-only file");
    return(0);
}
if(debug)
{
  fprintf(stderr, "-------\noufs_fwrite(%d)\n", len);   
}
if(oufs_read_inode_by_reference(fp->inode_reference, &inode) != 0) 
{
    return(-1);
}

BLOCK master_block;
virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &master_block); // Read in master block
BLOCK file_block; // File block to that will contain a buffer

  // Compute the index for the last block in the file + the first free byte within the block
  
  int current_blocks = fp->offset/DATA_BLOCK_SIZE;
  int used_bytes_in_last_block = fp->offset % DATA_BLOCK_SIZE;
  int free_bytes_in_last_block = DATA_BLOCK_SIZE - used_bytes_in_last_block;
  int len_written = 0;
BLOCK_REFERENCE new_block_ref;
BLOCK_REFERENCE last = UNALLOCATED_BLOCK;
BLOCK_REFERENCE next;
next = inode.content;
while(next != UNALLOCATED_BLOCK)
{
	last = next;
	virtual_disk_read_block(next,&file_block);
	next = file_block.next_block;
}

if(used_bytes_in_last_block == 0)
{
	free_bytes_in_last_block = 0; // Buffer is full
}
if(free_bytes_in_last_block > 0)
{
	memcpy(file_block.content.data.data + used_bytes_in_last_block,buf,MIN(len,free_bytes_in_last_block));
	len_written += MIN(len,free_bytes_in_last_block);
	buf += len_written;
	len -= len_written;
	virtual_disk_write_block(last,&file_block);
}
BLOCK new_block;
BLOCK_REFERENCE last_ref = last;
while(len > 0)
{
	fp->n_data_blocks += 1;
	new_block_ref = oufs_allocate_new_block(&master_block,&new_block); 
	if(inode.content == UNALLOCATED_BLOCK)
	{
		inode.content = new_block_ref;
	}
	else
	{		
		virtual_disk_read_block(last_ref,&new_block);
		new_block.next_block = new_block_ref; 
		virtual_disk_write_block(last_ref,&new_block);
	}
	virtual_disk_read_block(new_block_ref,&new_block); 
	new_block.next_block = UNALLOCATED_BLOCK; 
	memcpy(new_block.content.data.data,buf,MIN(len,DATA_BLOCK_SIZE));

	len_written += MIN(len,DATA_BLOCK_SIZE);
	buf += MIN(len,DATA_BLOCK_SIZE);

	len -= MIN(len,DATA_BLOCK_SIZE);
	virtual_disk_write_block(new_block_ref,&new_block);
	last_ref = new_block_ref;
}
if(fp->n_data_blocks == MAX_BLOCKS_IN_FILE)
{
	fprintf(stderr,"Error");
	return(-1000);
}
inode.size += len_written;
fp->offset += len_written;
oufs_write_inode_by_reference(fp->inode_reference,&inode);
virtual_disk_write_block(MASTER_BLOCK_REFERENCE,&master_block);

  // Done
  return(len_written);
}


/*
 * Read a sequence of bytes from an open file.
 * - offset is the current position within the file, and will never be larger than size
 * - offset will be updated with each read operation
 *
 * @param fp OUFILE pointer (must be opened for r)
 * @param buf Character buffer to place the bytes into
 * @param len Number of bytes to read at max
 * @return The number of bytes read
 *         0 if offset is at size
 *         -x if an error
 * 
 */

int oufs_fread(OUFILE *fp, unsigned char * buf, int len)
{

  INODE inode;
  BLOCK block;
  // Check open mode
if(fp->mode != 'r') 
{
    fprintf(stderr, "Can't read from a write-only file");
    return(0);
}
if(debug)
{
  fprintf(stderr, "\n-------\noufs_fread(%d)\n", len);
}
if(oufs_read_inode_by_reference(fp->inode_reference, &inode) != 0) 
{
    return(-1);
}
if(inode.size == 0)
{
	fprintf(stderr,"Nothing to be read\n");
	exit(-1);
} 
if(inode.content == UNALLOCATED_BLOCK)
{
	fprintf(stderr,"No block\n");
	exit(-1);
}
  // Compute the current block and offset within the block
  int current_block = fp->offset / DATA_BLOCK_SIZE;
  int byte_offset_in_block = fp->offset % DATA_BLOCK_SIZE;
  int len_read = 0;
  int free_bytes = DATA_BLOCK_SIZE - byte_offset_in_block; 
  int end_of_file = inode.size;
  len = MIN(len, end_of_file - fp->offset);
  int len_left = len;

fprintf(stderr,"Current_Block = %d\n", current_block);
fprintf(stderr,"Byte_offset_in_block = %d\n", byte_offset_in_block);
fprintf(stderr,"EOF = %d\n", end_of_file);
fprintf(stderr,"Len = %d\n", len);
fprintf(stderr,"fp->offset = %d\n", fp->offset);

BLOCK_REFERENCE next;
BLOCK_REFERENCE last;
BLOCK file_block;
if(fp->offset == end_of_file)
{
	return(0);
}
while(next != UNALLOCATED_BLOCK)
{
	last = next;
	virtual_disk_read_block(next,&file_block);
	next = file_block.next_block;
}

if(byte_offset_in_block > 0)
{
	memcpy(buf + byte_offset_in_block,file_block.content.data.data,byte_offset_in_block);
//	buf += byte_offset_in_block;
	len_read += MIN(len_left,byte_offset_in_block);
	len_left -= MIN(len_left,byte_offset_in_block);

}
BLOCK_REFERENCE last_ref;
while(len_left > 0)
{
	if(inode.content == UNALLOCATED_BLOCK)
	{
		inode.content = last_ref;
	}
	virtual_disk_read_block(inode.content,&file_block);
	fprintf(stderr,"next(before) = %d\n", next);
	memcpy(buf,file_block.content.data.data,MIN(len_left,DATA_BLOCK_SIZE));
	buf += MIN(len_left,DATA_BLOCK_SIZE);
	len_read += MIN(len_left,DATA_BLOCK_SIZE);
	len_left -= MIN(len_left,DATA_BLOCK_SIZE);
	if(byte_offset_in_block == 0)
	{
		last_ref = inode.content;
		inode.content = file_block.next_block;
	}
	fprintf(stderr,"next(after) = %d\n", next);
	
}

fprintf(stderr,"len_read = %d\n", len_read);
fp->offset += len_read;
//BLOCK file_block;
//virtual_disk_read_block(inode.content,&file_block);

//if(free_bytes > 0)
//{
//	memcpy(buf + byte_offset_in_block,file_block.content.data.data,MIN(len,free_bytes));
//	len_read += MIN(len,free_bytes);
	
//	*file_block.content.data.data += MIN(len,free_bytes);
//	len -= MIN(len,free_bytes);
//}

//fp->offset += len_read;
 // TODO

  // Done
  return(len_read);
}


/**
 * Remove a file
 *
 * Full implementation:
 * - Remove the directory entry
 * - Decrement inode.n_references
 * - If n_references == 0, then deallocate the contents and the inode
 *
 * @param cwd Absolute path for the current working directory
 * @param path Absolute or relative path of the file to be removed
 * @return 0 if success
 *         -x if error
 *
 */

int oufs_remove(char *cwd, char *path)
{
  INODE_REFERENCE parent;
  INODE_REFERENCE child;
  char local_name[MAX_PATH_LENGTH];
  INODE inode;
  INODE inode_parent;
  BLOCK parent_block;

  // Try to find the inode of the child
if(oufs_find_file(cwd, path, &parent, &child, local_name) < -1) 
{
    return(-3);
};
  
if(child == UNALLOCATED_INODE) 
{
    fprintf(stderr, "File not found\n");
    return(-1);
}
  // Get the inode
if(oufs_read_inode_by_reference(child, &inode) != 0) 
{
    return(-4);
}

  // Is it a file?
if(inode.type != FILE_TYPE) 
{
    // Not a file
    fprintf(stderr, "Not a file\n");
    return(-2);
}
oufs_read_inode_by_reference(parent,&inode_parent);// Read in parent inode
BLOCK master_block;
virtual_disk_read_block(MASTER_BLOCK_REFERENCE,&master_block);
virtual_disk_read_block(inode_parent.content,&parent_block); // Read in parent block
fprintf(stderr,"inode.content = %d\n", inode.content);

int byte = child / 8; // Find byte location of INODE_REFERENCE
int bit = 7 - (child % 8);//Find bit location within bit array
oufs_deallocate_blocks(&inode);
//oufs_deallocate_block(&master,inode.content);
for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++)
{
	if(strcmp(parent_block.content.directory.entry[i].name,local_name) == 0)
	{
		parent_block.content.directory.entry[i].inode_reference = UNALLOCATED_INODE; // Set that inode to unallocated_inode, effectively removing it
	}
}
inode_parent.size -= 1;
virtual_disk_read_block(MASTER_BLOCK_REFERENCE,&master_block);
master_block.content.master.inode_allocated_flag[byte] ^= 1 << bit; // Flip bit back to 0
virtual_disk_write_block(inode_parent.content,&parent_block);
virtual_disk_write_block(MASTER_BLOCK_REFERENCE,&master_block);
oufs_write_inode_by_reference(parent,&inode_parent);



  // TODO
  
  // Success
  return(0);
};


/**
 * Create a hard link to a specified file
 *
 * Full implemenation:
 * - Add the new directory entry
 * - Increment inode.n_references
 *
 * @param cwd Absolute path for the current working directory
 * @param path_src Absolute or relative path of the existing file to be linked
 * @param path_dst Absolute or relative path of the new file inode to be linked
 * @return 0 if success
 *         -x if error
 * 
 */
int oufs_link(char *cwd, char *path_src, char *path_dst)
{
  INODE_REFERENCE parent_src;
  INODE_REFERENCE child_src;
  INODE_REFERENCE parent_dst;
  INODE_REFERENCE child_dst;
  char local_name[MAX_PATH_LENGTH];
  char local_name_bogus[MAX_PATH_LENGTH];
  INODE inode_src;
  INODE inode_dst;
  BLOCK block;

  // Try to find the inodes
if(oufs_find_file(cwd, path_src, &parent_src, &child_src, local_name_bogus) < -1) 
{
    return(-5);
}
if(oufs_find_file(cwd, path_dst, &parent_dst, &child_dst, local_name) < -1) 
{
    return(-6);
}

  // SRC must exist
if(child_src == UNALLOCATED_INODE) 
{
    fprintf(stderr, "Source not found\n");
    return(-1);
}

  // DST must not exist, but its parent must exist
if(parent_dst == UNALLOCATED_INODE) 
{
    fprintf(stderr, "Destination parent does not exist.\n");
    return(-2);
}
if(child_dst != UNALLOCATED_INODE) 
{
    fprintf(stderr, "Destination already exists.\n");
    return(-3);
}

  // Get the inode of the dst parent
if(oufs_read_inode_by_reference(parent_dst, &inode_dst) != 0) 
{
    return(-7);
}

if(inode_dst.type != DIRECTORY_TYPE) 
{
    fprintf(stderr, "Destination parent must be a directory.");
}
  // There must be space in the directory
if(inode_dst.size == N_DIRECTORY_ENTRIES_PER_BLOCK) 
{
    fprintf(stderr, "No space in destination parent.\n");
    return(-4);
}
INODE parent_src_inode;
oufs_read_inode_by_reference(parent_src,&parent_src_inode);
if(&inode_src.type == DIRECTORY_TYPE)
{
	fprintf(stderr,"Noooo\n");
	return(-1);
}
INODE child_src_inode;
oufs_read_inode_by_reference(child_src,&child_src_inode);
BLOCK parent_src_block;
BLOCK parent_dst_block;

if(child_src_inode.type == DIRECTORY_TYPE)
{
	fprintf(stderr,"Nooo again\n");
	return(-2);
}

virtual_disk_read_block(inode_dst.content,&parent_dst_block);
for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++)
{
	if(parent_dst_block.content.directory.entry[i].inode_reference == UNALLOCATED_INODE)
	{
		inode_dst.size += 1;
		child_src_inode.n_references += 1;
		fprintf(stderr,"Made it in\n");
		parent_dst_block.content.directory.entry[i].inode_reference = child_src; // Set that inode to unallocated_inode, effectively deallocating it
		strcpy(parent_dst_block.content.directory.entry[i].name, local_name);
		break;	
	}
}
oufs_write_inode_by_reference(child_src,&child_src_inode);
oufs_write_inode_by_reference(parent_dst,&inode_dst);
virtual_disk_write_block(inode_dst.content,&parent_dst_block);
  // TODO
}


