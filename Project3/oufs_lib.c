/**
 *  Project 3
 *  oufs_lib.c
 *
 *  Author: CS3113
 *
 */

#include "oufs_lib.h"
#include "oufs_lib_support.h"
#include "virtual_disk.h"

// Yes ... a global variable
int debug = 1;

// Translate inode types to descriptive strings
const char *INODE_TYPE_NAME[] = {"UNUSED", "DIRECTORY", "FILE"};

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
if(str == NULL) 
{
    	// Provide default
	strcpy(cwd, "/");
}
	else
	{	
    		// Exists
		strncpy(cwd, str, MAX_PATH_LENGTH-1);
	}

  	// Virtual disk location
	str = getenv("OUFS_DISK");

if(str == NULL) 
{
    	// Default
	strcpy(disk_name, "vdisk1");
}
	else
	{
    		// Exists: copy
		strncpy(disk_name, str, MAX_PATH_LENGTH-1);
	}

  // Pipe name base
str = getenv("OUFS_PIPE_NAME_BASE");

if(str == NULL) 
{
    	// Default
	strcpy(pipe_name_base, "pipe");
}
	else
	{
    		// Exists: copy
		strncpy(pipe_name_base, str, MAX_PATH_LENGTH-1);
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
if(virtual_disk_attach(virtual_disk_name, pipe_name_base) != 0) 
{
    return(-1);
}

BLOCK block;

  // Zero out the block
memset(&block, 0, BLOCK_SIZE);

for(int i = 0; i < N_BLOCKS; ++i) 
{
	if(virtual_disk_write_block(i, &block) < 0) 
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
if(oufs_write_inode_by_reference(0, &inode) != 0) 
{
    return(-3);
}

virtual_disk_write_block(ROOT_DIRECTORY_BLOCK, &block); // Write the root block

  // TODO: complete implementation
    

memset(&block, 0, BLOCK_SIZE);// Clear out blocks again

//////////////////////////////////////////////////////
 // All other blocks are free blocks

for(BLOCK_REFERENCE  i = 7; i < N_BLOCKS; i++)
{
	block.next_block = i;
	virtual_disk_write_block( i - 1 , &(block)); // Write free blocks
	if( i == 127)
	{
		block.next_block = UNALLOCATED_BLOCK;
		virtual_disk_write_block(i,&block);
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
DIRECTORY_ENTRY* e1 = (DIRECTORY_ENTRY*) d1;
DIRECTORY_ENTRY* e2 = (DIRECTORY_ENTRY*) d2;
int result = strcmp(e1->name,e2->name); // assign result to what ever strcmp returns


if((e1->inode_reference != UNALLOCATED_INODE) && (e2->inode_reference != UNALLOCATED_INODE))
{
	return (result);
}
else if((e1->inode_reference == UNALLOCATED_INODE)&&(e2->inode_reference != UNALLOCATED_INODE))
{
	return (-1);
}

else if((e1->inode_reference != UNALLOCATED_INODE)&&(e2->inode_reference == UNALLOCATED_INODE))
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
	if(ret == 0 && child != UNALLOCATED_INODE) 
	{
    	// Element found: read the inode
		INODE inode;
		if(oufs_read_inode_by_reference(child, &inode) != 0) 
		{
      			return(-1);
		}
		if(debug)
		{
			fprintf(stderr, "\tDEBUG: Child found (type=%s).\n",  INODE_TYPE_NAME[inode.type]);
		}

		BLOCK b;
		virtual_disk_read_block(inode.content,&b);
		//Read in directory block to list
		qsort(b.content.directory.entry, N_DIRECTORY_ENTRIES_PER_BLOCK,sizeof(DIRECTORY_ENTRY),inode_compare_to);
		// sort the entries
		//TODO Find out how to compare directories and list their contents(Marcos)
	for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++)
	{
		if(inode.type == DIRECTORY_TYPE && b.content.directory.entry[i].inode_reference != UNALLOCATED_INODE) // Inode must exist
		{
			printf("%s/\n", b.content.directory.entry[i].name); // Print entry
		}
		
	}	
		
    	// TODO: complete implementation

	}
	else
	{
    // Did not find the specified file/directory
		fprintf(stderr, "Not found\n");
		if(debug)
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
fprintf(stderr,"Before find fill\n");
  // Attempt to find the specified directory
if((ret = oufs_find_file(cwd, path, &parent, &child, local_name)) < -1) 
{
	if(debug)
	{
		fprintf(stderr, "oufs_mkdir(): ret = %d\n", ret);
	}
    return(-1);
};

if(child != UNALLOCATED_INODE || parent == UNALLOCATED_INODE )
{
	fprintf(stderr,"Child is Allocated or parent is not\n");
	return(-1);
}


BLOCK b; // Block we just initiated
INODE inode; // Inode we just initiated
INODE_REFERENCE child_inode_ref; 

fprintf(stderr,"Before allocate new dir call\n");

fprintf(stderr,"Child is: %d, Parent is: %d\n", child, parent);

oufs_read_inode_by_reference(parent,&inode);//Read the parent inode

virtual_disk_read_block(inode.content,&b); // Parent block

child_inode_ref =  oufs_allocate_new_directory(parent); // Get inode Reference and allocated a new directory
//oufs_allocate_new_directory(parent); // Not sure

for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++)
{
	if(inode.size <= N_DIRECTORY_ENTRIES_PER_BLOCK && inode.type == DIRECTORY_TYPE)
	{
		if(b.content.directory.entry[i].inode_reference == UNALLOCATED_INODE)
		{
			strcpy(b.content.directory.entry[i].name,local_name); // Set name
			b.content.directory.entry[i].inode_reference = child_inode_ref; // Set inode_reference
			break;

		}
	}
	else
	{
		fprintf(stderr,"Not enough space or not a DIRECTORY_TYPE\n");
	}
}

//parent = inode_reference;

inode.size += 1;//Update the size

oufs_write_inode_by_reference(parent,&inode); // Write contents of inode to disk
virtual_disk_write_block(inode.content,&b); // Write Block contents to disk
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
if(oufs_find_file(cwd, path, &parent, &child, local_name) < -1) 
{
    return(-4);
}

if(child == UNALLOCATED_INODE)
{
	fprintf(stderr,"Child should not be UNALLOCATED!\n");
}


BLOCK master_block;
virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &master_block);

BLOCK parent_block;
INODE parent_inode;
INODE child_inode;
BLOCK child_block;

oufs_read_inode_by_reference(parent,&parent_inode); // Read in parent inode
oufs_read_inode_by_reference(child,&child_inode); // Read in child inode

virtual_disk_read_block(parent_inode.content,&parent_block); // Read in child block
virtual_disk_read_block(child_inode.content,&child_block);// Read in child block

if(child_inode.size > 2) // Still directories in child, cannot delete entries within parent
{
		return(-3);
}
if((strcmp(local_name, ".")) == 0) // Cannot delete .
{
	return(-8);
}
if((strcmp(local_name, "..")) == 0) // Cannot delete ..
{
	return(-3);
}
if(child_inode.content == UNALLOCATED_BLOCK) // Block should exist
{
	return(-3);
}
int byte = child/8; // Find byte location of INODE_REFERENCE
int bit = 7 - (child%8) ;//Find bit location within bit array

fprintf(stderr,"Child is: %d\n ", child);
fprintf(stderr,"Parent is: %d\n",parent);
fprintf(stderr,"byte is: %d\n",byte);
fprintf(stderr,"bit is: %d\n",bit);
fprintf(stderr,"Parent size is: %d\n", parent_inode.size);
fprintf(stderr,"Child size is: %d\n", child_inode.size);

oufs_deallocate_block(&master_block,child_inode.content);
for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++)
{
	if((strcmp(parent_block.content.directory.entry[i].name,local_name) == 0))// Look for name if it even exists
	{
		fprintf(stderr,"Everything should be removed\n");
		parent_block.content.directory.entry[i].inode_reference = UNALLOCATED_INODE; // Set that inode to unallocated_inode, effectively deallocating it
	}
}

		parent_inode.size -= 1;	//Decrement size of newly deallocated entry within parent
		master_block.content.master.inode_allocated_flag[byte] ^= 1 << bit; // Flip bit back to 0
		oufs_write_inode_by_reference(parent,&parent_inode); // Write out parent inode
		virtual_disk_write_block(parent_inode.content,&parent_block); // Write parent block

		virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &master_block); // Write master block
		virtual_disk_write_block(child_inode.content,&child_block);
		oufs_write_inode_by_reference(child,&child_inode); // Write out child inode
  // TODO: complet implementation


  // Success
  return(0); // Yay
}
