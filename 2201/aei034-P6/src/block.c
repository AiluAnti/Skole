
#include "block.h"
#include "fs.h"

#include "common.h"
#include "util.h"
#include "usb/scsi.h"


extern const int os_size;

/*
 * block_init:
 * Initialize the block code. For USB access, no initialization is
 * needed. However, it exists so that the interface is the same as for
 * the block_sim code.
 * 
 */
void block_init(void)
{
    /* We assume that block_size == sector size */
    ASSERT(BLOCK_SIZE == SECTOR_SIZE);
}


/*
 * block_destruct:
 * Cleanup for the block code. For USB access, no cleanup is
 * needed. However, it exists so that the interface is the same as for
 * the block_sim code.
 * 
 */
void block_destruct(void)
{
    /* Nothing to do */
}

/*
 * block_read:
 * Reads a disk block (512 bytes) from block_num 
 * into the memory pointed to by address.
 */
int block_read(int block_num, void *address)
{
    char addr_buf[BLOCK_SIZE];
    int ret;

    ret = scsi_read(block_num, 1, addr_buf);

    if (ret < 0)
        return -1;

    bcopy(addr_buf, (char *) address, BLOCK_SIZE);

    
    return 0;
}


/*
 * block_write:
 * Writes the 512 bytes starting at address to the disk block
 * block_num 
 */
int block_write(int block_num, void *address)
{
    if (!block_num || address == NULL)
	   return -1;
    return scsi_write(block_num, 1, address);
}

/*
 * block_modify:
 * Changes a part of a disk block. The block block_num is changed so
 * that the part of the block from offset until offset+data_size is
 * replaced with the first data_size bytes from data.
 */
int block_modify(int block_num, int offset, void *data, int data_size)
{
    int rc;
    char buf[BLOCK_SIZE];

    ASSERT((offset + data_size) <= BLOCK_SIZE);

    rc = block_read(block_num, buf);
    if (rc == 0) 
    {
	   bcopy(data, &buf[offset], data_size);
	   rc = block_write(block_num, buf);
    } 
    else
	   return -1;

    return rc;
}

/*
 * block_read_part:
 * Read a part of a disk block. The data from the disk block block_num
 * starting at offset until offset+bytes is read into the memory
 * starting at address.
 */
int block_read_part(int block_num, int offset, int bytes, void *address)
{
    int rc;
    char buf[BLOCK_SIZE];

    ASSERT((offset + bytes) <= BLOCK_SIZE);

    rc = block_read(block_num, buf);
    if (rc == 0) {
	bcopy(&(buf[offset]), address, bytes);
	return 0;
    }

    return -1;
}
