#include "fs.h"

#ifdef LINUX_SIM
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#else
#define printf rsprintf
#endif				/* LINUX_SIM */

#include "common.h"
#include "block.h"
#include "util.h"
#include "thread.h"
#include "inode.h"
#include "superblock.h"
#include "kernel.h"
#include "fs_error.h"

#define BITMAP_ENTRIES 256
#define INODE_TABLE_ENTRIES 20
#define INODE_BLOCKS ((BITMAP_ENTRIES / INODE_TABLE_ENTRIES) + 1)

static unsigned char inode_bmap[BITMAP_ENTRIES];
static unsigned char dblk_bmap[BITMAP_ENTRIES];

static int get_free_entry(unsigned char *bitmap);
static int free_bitmap_entry(int entry, unsigned char *bitmap);
static inode_t name2inode(const char *name, inode_t num);
static blknum_t ino2blk(inode_t ino);
static blknum_t idx2blk(int index);
static void create_dir(inode_t inode_num);
static int get_free_fd();
static void update_pcb(inode_t num, int fd, int mode);
static void update_inode(struct disk_inode curr, inode_t num);
static struct disk_inode read_inode(inode_t num);
static int parse_path(char *buf, const char *path);
static int mkfile(const char *filename);
static void create_file(inode_t inode_num);
static int ino_offset(inode_t num);


struct mem_superblock m_super;
struct disk_superblock *d_super = &m_super.d_super;

/* Write the updated bitmaps */
static void update_bitmaps(void)
{
    block_modify(d_super->root_inode - SUPERBLK_SIZE, 0, m_super.ibmap, sizeof(inode_bmap));
    block_modify(d_super->root_inode - SUPERBLK_SIZE, sizeof(inode_bmap), m_super.dbmap, sizeof(dblk_bmap));   
}

/* Write inode to inode table at the right index */
static void update_inode(struct disk_inode curr, inode_t num)
{
    block_modify(d_super->root_inode + ino2blk(num),
                 ino_offset(num),
                 &curr, sizeof(curr));
}

/* Returns an offset based off of an inode number */
static int ino_offset(inode_t num)
{
    return (sizeof(struct disk_inode) * num) % BLOCK_SIZE;
}


/* Reads a disk inode from disk */
static struct disk_inode read_inode(inode_t num)
{
    struct disk_inode tmp;

    block_read_part(d_super->root_inode + ino2blk(num), ino_offset(num), sizeof(struct disk_inode), &tmp);

    return tmp;   
}
/*
 * Exported functions.
 */
void fs_init(void)
{
    block_init();
    /* Check whether superblock is on disc */
    block_read_part(os_size + 2, 0, sizeof(*d_super), d_super);
    /* Bitmaps */
    m_super.ibmap = inode_bmap;
    m_super.dbmap = dblk_bmap;
    m_super.dirty = 0;
    /* Check whether magic signature exists */
    if (d_super->magic == EXT2_SUPER_MAGIC)
    {
        /* Read the inode bitmap */
        block_read_part(d_super->root_inode - SUPERBLK_SIZE, 0, sizeof(inode_bmap), inode_bmap);
        /* Read the data block bitmap */
        block_read_part(d_super->root_inode - SUPERBLK_SIZE, sizeof(inode_bmap),
                        sizeof(dblk_bmap), dblk_bmap);
    }
    else
    {
        /* Make new fs */
        fs_mkfs();
    }
}

/*
 * Make a new file system.
 * Argument: kernel size
 */
void fs_mkfs(void)
{
    struct mem_inode tmp;
    
    d_super->ninodes = BITMAP_ENTRIES; /* 256 inodes */
    d_super->ndata_blks = BITMAP_ENTRIES; /* 256 data blocks */
    d_super->root_inode = os_size + 4; /* root inode will be directly after the bitmap blocks */
    d_super->max_filesize = BLOCK_SIZE * (INODE_NDIRECT); /* Max filesize without indirection */
    d_super->magic = EXT2_SUPER_MAGIC; /* Magic signature  */

    // Assign an inode num for the root inode
    current_running->cwd = get_free_entry(m_super.ibmap);
    // We want to read/write from pos 0
    tmp.d_inode = read_inode(current_running->cwd);
    tmp.pos = 0;

    // Write the superblock
    block_modify(d_super->root_inode - SUPERBLK_SIZE - INODE_BLK_SIZE, 0, d_super, sizeof(*d_super));

    // Write the updated inode back to disc
    update_inode(tmp.d_inode, 0);
    update_bitmaps();

    // Make what will be the root directory
    create_dir(current_running->cwd);
}

/* This function might not be very well named, and what it actually does is really nothing
 * compared to the amount of arguments it requires. It's only made to save line space */
static void update_pcb(inode_t num, int fd, int mode)
{
    current_running->filedes[fd].idx = num;
    current_running->filedes[fd].mode = mode;
    current_running->filedes[fd].pos = 0;
}

int fs_open(const char *filename, int mode)
{
    /* Buf to store what is parsed */
    char buf[MAX_FILENAME_LEN];

    int tmp;
    int fd;
    int i;
    int ino_num;

    /* Get free filedes index */
    fd = get_free_fd();
    /* This should always be an absolute path */

    struct mem_inode curr_dir;
    
    if (filename[0] != '/')
    {
        if (mode & (MODE_WRONLY | MODE_CREAT | MODE_TRUNC) || mode & (MODE_RDONLY))
        {
            // Get inode num of current dir
            ino_num = name2inode(filename, current_running->cwd);;

            if (ino_num < 0)
            {
                // If trying to stat something that doesn't exist
                if ((mode & MODE_CREAT) == 0)
                    return FSE_NOTEXIST;

                // Create the new file, because create bit is set and file isn't found
                mkfile(filename);
                
                // Assign new ino num as curr ino num
                ino_num = name2inode(filename, current_running->cwd);

                update_pcb(ino_num, fd, mode);
                return fd;
            }
            else if(ino_num > 0)
            {
                // Read current inode
                curr_dir.d_inode = read_inode(ino_num);
                
                // If it's not a file, return error
                if(curr_dir.d_inode.type != INTYPE_FILE)
                {
                    return FSE_INVALIDNAME;
                }

                update_pcb(ino_num, fd, mode);
                return fd;
            }
        }
        // If mode is none of the above
        return -1;        
    }

    if (filename[1] == '\0')
    {
        /* Return root dir */
        curr_dir.d_inode = read_inode(0);

        update_pcb(0, fd, mode);
        return fd;
    }
    else
    {
        /* Using tmp to store curr dir when func is called */
        curr_dir.d_inode = read_inode(0);

        /* Read root inode into curr dir */
        curr_dir.inode_num = 0;

        /* tmp that gives res from parse without adding 1 for the first '/' */
        tmp = parse_path(buf, &filename[1]);
        /* int that will tell us where to keep parsing from */
        i = parse_path(buf, &filename[1]) + 1;

        while (tmp > 0)
        {
            /* While res from parsing is > 0 */
            ino_num = name2inode(buf, curr_dir.inode_num);

            if (ino_num > 0)
            {
                curr_dir.inode_num = ino_num;
                
                curr_dir.d_inode = read_inode(ino_num);

                tmp = parse_path(buf, &filename[i]);
                if (tmp > 0)
                    i += tmp;
                else if(i > MAX_PATH_LEN)
                    return FSE_NAMETOLONG;
            }
            else
                break;
        }  

        if (tmp == 0)
        {
            /* While res from parsing is > 0 */
            ino_num = name2inode(buf, curr_dir.inode_num);

            if (ino_num < 0)
            {
                return FSE_PARSEERROR;
            }
            // Curr inode is last one in buf                        
            curr_dir.d_inode = read_inode(ino_num);
            
            update_pcb(ino_num, fd, mode);
            return fd;
        }
        else
        {
            return FSE_PARSEERROR;
        }
    }
}

int fs_read(int fd, char *buffer, int size)
{
    struct mem_inode curr;
    curr.d_inode = read_inode(current_running->cwd);
    //curr.pos = current_running->filedes[fd].pos;
    int idx, idx_blk, offset, j;

    if (fd < 0 || fd >= MAX_OPEN_FILES)
    {
        return FSE_INVALIDHANDLE;
    }

    // In this case, there is nothing more to read
    if (curr.d_inode.size == current_running->filedes[fd].pos)
    {
        return FSE_OK;
    }

    if(current_running->filedes[fd].mode & (MODE_RDONLY | MODE_RDWR))
    {
        // Read open file or dir
        curr.d_inode = read_inode(current_running->filedes[fd].idx);

        // Idx within a block
        idx = current_running->filedes[fd].pos / BLOCK_SIZE;

        if (idx > INODE_NDIRECT - 1)
            return FSE_FULL;

        // The actual block we want to read from
        idx_blk = idx2blk(curr.d_inode.direct[idx]);

        // Offset within the block
        offset = current_running->filedes[fd].pos % BLOCK_SIZE;
        
        if(curr.d_inode.type == INTYPE_FILE)
        {
            // Don't read beyond the size of the file
            if(size > curr.d_inode.size)
                size = curr.d_inode.size;
            if (offset + size > BLOCK_SIZE)
            {
                // J is the last bytes of the current block
                j = BLOCK_SIZE - offset;

                // Read the last j bytes in the first block into buffer
                block_read_part(d_super->root_inode + idx_blk, offset, j, buffer);

                // Read the remaining bytes into buffer
                block_read_part(d_super->root_inode + idx2blk(curr.d_inode.direct[idx+1]),
                                0, size - j, &(buffer[j]));
            }
            else
            {
                // Read size bytes into buffer
                block_read_part(d_super->root_inode + idx_blk, offset, size, buffer);
            }
        }
        else
        {
            if (offset + size > BLOCK_SIZE)
            {
                // J is the last bytes of the current block
                j = BLOCK_SIZE - offset;

                // Read the last j bytes in the first block into buffer
                block_read_part(d_super->root_inode + idx_blk, offset, j, buffer);

                // Read the remaining bytes into buffer
                block_read_part(d_super->root_inode + idx2blk(curr.d_inode.direct[idx+1]),
                                0, size - j, &(buffer[j]));
            }
            else
            {
                // Read size bytes into buffer
                block_read_part(d_super->root_inode + idx_blk, offset, size, buffer);
            }
        }
    }
    else
    {
        return FSE_INVALIDMODE;
    }
         

    // Update position based on current position compared to size
    if (current_running->filedes[fd].pos > curr.d_inode.size)
    {
        current_running->filedes[fd].pos = curr.d_inode.size;
        curr.pos = current_running->filedes[fd].pos;
        return FSE_EOF;
    }
    else if(current_running->filedes[fd].pos == curr.d_inode.size) {
        current_running->filedes[fd].pos = 0;
        curr.pos = current_running->filedes[fd].pos;
        return FSE_OK;
    }
    else
    {
        current_running->filedes[fd].pos += size;
        curr.pos = current_running->filedes[fd].pos;
        return size;
    }
}

void create_dir(inode_t inode_num)
{
    int blk_idx, offset, tmp;
    /* New inode */
    struct disk_inode inode;

    /* Assign type to the dir */
    inode.type = INTYPE_DIR;
    
    inode.nlinks = 0;

    /* Pointer to the first data block */
    inode.direct[0] = get_free_entry(m_super.dbmap);

    if (inode.direct[0] < 0)
        return FSE_BITMAP;

    inode.size = sizeof(struct dirent) * 2;
    
    /* Define the current and parent dir for the dir */
    struct dirent dir[2] = {    [0].inode = inode_num, [0].name = ".",
                                [1].inode = current_running->cwd, [1].name = ".." };


    // Where to write
    blk_idx = d_super->root_inode + ino2blk(inode_num);
    
    // Find out whether we need to allocate a new block
    offset = (sizeof(inode) * inode_num) % BLOCK_SIZE;
    
    if (offset + sizeof(inode) > BLOCK_SIZE)
    {
        if (inode_num * sizeof(inode) / BLOCK_SIZE > INODE_BLOCKS)
            return FSE_NOMOREINODES;

        // How much we can write on the first block
        tmp = BLOCK_SIZE - offset;
        
        // Write tmp bytes to first block
        block_modify(d_super->root_inode + ino2blk(inode_num), 
                offset, &inode, tmp);

        // Write the rest to the next block
        block_modify(d_super->root_inode + ino2blk(inode_num) + 1, 
                0, &(((char *)&inode)[tmp]), sizeof(inode) - tmp);
    }
    else
    {
        // Write size to block
        block_modify(d_super->root_inode + ino2blk(inode_num), 
                    offset, &inode, sizeof(inode));
    }

    /* Write the data block containing "." and ".." */
    block_modify(d_super->root_inode + idx2blk(inode.direct[0]), 0, &dir, sizeof(dir));
}

int fs_mkdir(char *dirname)
{
    /* Create new dir entry for parent dir */
    struct dirent dir_entry;
    struct disk_inode curr; 
    
    int blk_idx, i, j;
    /* Allocate free inode in bitmap */
    dir_entry.inode = get_free_entry(inode_bmap);
    
    if (dir_entry.inode < 0)
        return FSE_BITMAP;
    
    // Create new dir with newly assigned inode number
    create_dir(dir_entry.inode);

    /* Copy dirname to dir entry name */
    strcpy(dir_entry.name, dirname);

    
    /* curr is the parent dir from which we make the new dir */
    curr = read_inode(current_running->cwd);
    
    /* If inode size + size of new dir entry is too big, return error */
    if (curr.size + sizeof(dir_entry) > d_super->max_filesize)
            return FSE_ERROR;

    /* blk_idx finds the index within a given inode */
    blk_idx = curr.direct[curr.size / BLOCK_SIZE];
    
    // The data block index to which we want to write
    i = ((curr.size + sizeof(struct dirent))/BLOCK_SIZE);
    
    if (i > INODE_NDIRECT - 1)
        return FSE_FULL;

    /* If this triggers, a new index in the inode is needed */
    if ((curr.size % BLOCK_SIZE) + sizeof(dir_entry) > BLOCK_SIZE)
    {
        // Allocate a new datablock
        curr.direct[i] = get_free_entry(m_super.dbmap);

        // The amount of bytes we need to write to first block
        j = BLOCK_SIZE - (curr.size % BLOCK_SIZE);

        // Write j bytes to the first block
        block_modify(d_super->root_inode + idx2blk(curr.direct[i - 1]), curr.size % BLOCK_SIZE, &dir_entry, j);

        // Write the rest to the next block
        block_modify(d_super->root_inode + idx2blk(curr.direct[i]) , 0, &((char *)&dir_entry)[j], sizeof(dir_entry) - j);

        /* Change size of parent dir */
        curr.size += sizeof(struct dirent);
        
        update_inode(curr, current_running->cwd);
        update_bitmaps();
    }
    else
    {
        // Index to which we want to write
        i = curr.size / BLOCK_SIZE;

        if (i >= INODE_NDIRECT - 1)
            return FSE_FULL;

        /* Write the dir entry to disk */
        block_modify(d_super->root_inode + idx2blk(curr.direct[i]), curr.size % BLOCK_SIZE,
            &dir_entry, sizeof(dir_entry));
        
        
        /* Change size of parent dir */
        curr.size += sizeof(struct dirent);
        
        update_inode(curr, current_running->cwd);
        update_bitmaps();
    }
    return FSE_OK;
}

static int mkfile(const char *filename)
{
    struct disk_inode curr;
    int i, blk_idx;

    /* Make sure filename is valid*/
    if (name2inode(filename, current_running->cwd) > 0)
        return FSE_EXIST;
    
    /* Create new dir entry for parent dir */
    struct dirent dir_entry;
    
    /* Allocate free inode in bitmap */
    dir_entry.inode = get_free_entry(inode_bmap);
    
    if (dir_entry.inode < 0)
        return FSE_BITMAP;

    /* Copy filename to dir entry name */
    strlcpy(dir_entry.name, filename, sizeof(dir_entry.name));

    // Create new file with new inode
    create_file(dir_entry.inode);

    // Current inode is parent 
    curr = read_inode(current_running->cwd);
    
    /* If inode size + size of new dir entry is too big, return error */
    if (curr.size + sizeof(dir_entry) > d_super->max_filesize)
            return FSE_ERROR;

    /* If this triggers, a new index in the inode is needed*/
    if (curr.size % BLOCK_SIZE + sizeof(dir_entry) > BLOCK_SIZE)
    {
        i = (curr.size/BLOCK_SIZE) + 1;
        
        if (i > INODE_NDIRECT)
            return FSE_FULL;
        
        curr.direct[i] = get_free_entry(m_super.dbmap);
    }
    
    /* blk_idx finds the index within a given inode */
    blk_idx = curr.direct[curr.size / BLOCK_SIZE];
    
    /* Write the dir entry to disk */
    block_modify(d_super->root_inode + idx2blk(blk_idx), curr.size % BLOCK_SIZE,
        &dir_entry, sizeof(dir_entry));

    /* Change size of current dir */
    curr.size += sizeof(dir_entry);

    update_inode(curr, current_running->cwd);
    update_bitmaps();
    return FSE_OK;
}

static void create_file(inode_t inode_num)
{
    /* Create new structs */
    struct disk_inode inode;

    /* Assign type to the dir */
    inode.type = INTYPE_FILE;

    inode.nlinks = 1;
    
    /* Pointer to the first block */
    inode.direct[0] = get_free_entry(m_super.dbmap);
    
    if (inode.direct[0] < 0)
        return FSE_FULL;

    inode.size = 0;

    /* Write inode to inode table at the right index */
    update_inode(inode, inode_num);
}


static int get_free_fd()
{
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (current_running->filedes[i].mode == MODE_UNUSED)
            return i;
    }
    return FSE_NOMOREFDTE;
}

static int parse_path(char *buf, const char *path)
{
    int i;

    /* Copy until end of path met, slash is met, or the filename is too long */
    for (i = 0; path[i] != '/' && path[i] != '\0' && i < (MAX_FILENAME_LEN - 1); i++) {
        buf[i] = path[i];
    }
    //puts("");

    /* Make sure buf is null-terminated */
    buf[i] = '\0';


    if(path[i] == '/') // There is more to be parsed, return next letter to parse from
    {
        return i + 1;
    }
    else if(path[i] == '\0') // End of path, 0-terminated aka parsing is done
    {
        return FSE_OK; 
    }
    else
    {
        return FSE_PARSEERROR;
    }
}

int fs_close(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES)
    {
        return FSE_INVALIDHANDLE;
    }
    current_running->filedes[fd].mode = MODE_UNUSED;
    return FSE_OK;
}


int fs_write(int fd, char *buffer, int size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES)
    {
        return FSE_INVALIDHANDLE;
    }

    struct mem_inode curr;
    int i, final_index, offset, tmp;





    // Read current inode
    curr.d_inode = read_inode(current_running->filedes[fd].idx);

    // Check whether mode is valid
    if(current_running->filedes[fd].mode & (MODE_WRONLY | MODE_CREAT | MODE_TRUNC))
    {
        // Offset from where we want to write        
        offset = curr.d_inode.size % BLOCK_SIZE;
        
        // If the write prompts a block-wrap, this is the last block to be written to
        final_index = (curr.d_inode.size + size) / BLOCK_SIZE; 
        
        if (final_index > INODE_NDIRECT - 1)
            return FSE_FULL;
        
        // If we need to block-wrap
        if ((curr.d_inode.size % BLOCK_SIZE) + size > BLOCK_SIZE)
        {
            // i will be the index from which we begin the writing
            for(i = (curr.d_inode.size/BLOCK_SIZE); i < final_index; i++)
                {
                    // The index we want to wrap to
                    curr.d_inode.direct[final_index] = get_free_entry(m_super.dbmap);
                    if (curr.d_inode.direct[final_index] < 0)
                        return FSE_BITMAP;
                    
                    // How much to write on the first block
                    tmp = BLOCK_SIZE - (curr.d_inode.size % BLOCK_SIZE);
                    
                    // Write tmp bytes to first block
                    block_modify(d_super->root_inode + idx2blk(curr.d_inode.direct[i]),
                             offset, buffer, tmp);

                    // Write the rest of the bytes to the last index
                    block_modify(d_super->root_inode + idx2blk(curr.d_inode.direct[final_index]),
                             0, &(buffer)[tmp], size - tmp);

                    update_inode(curr.d_inode, current_running->filedes[fd].idx);
                    update_bitmaps();
                    return FSE_OK;
                }


        }
        // If we are still on the first block
        else if(curr.d_inode.size < BLOCK_SIZE)
            i = 0;
        else
        {
            i = (curr.d_inode.size/BLOCK_SIZE);
        }
        
        // Write size bytes to buffer
        block_modify(d_super->root_inode + idx2blk(curr.d_inode.direct[i]),
                     offset, buffer, size);

        // Update size and pos
        curr.d_inode.size += size;
        current_running->filedes[fd].pos += size;
        
        update_inode(curr.d_inode, current_running->filedes[fd].idx);
        update_bitmaps();
        return FSE_OK;
    }
    return -1;        
}

/*
 * fs_lseek:
 * This function is really incorrectly named, since neither its offset
 * argument or its return value are longs (or off_t's). Also, it will
 * cause blocks to allocated if it extends the file (holes are not
 * supported in this simple filesystem).
 */
int fs_lseek(int fd, int offset, int whence)
{
    return -1;
}



int fs_chdir(char *path)
{
    char buf[MAX_FILENAME_LEN];
    int index, tmp, ino_num;

    struct mem_inode curr;
    curr.d_inode = read_inode(current_running->cwd);

    
    if(path[0] == '/')
    {

        // Make current dir root inode
        curr.d_inode = read_inode(0);
        
        // Cwd is root   
        current_running->cwd = 0;

        index = parse_path(buf, &path[1]);
        tmp = parse_path(buf, &path[1]) + 1;

        // Retrieve inode number from first subdirectory
        ino_num = name2inode(buf, current_running->cwd);

        while (index > 0)
        {
            ino_num = name2inode(buf, current_running->cwd);

            if (ino_num < 0)
                return FSE_DENOTFOUND;

            index = parse_path(buf, &path[tmp]);
            tmp += index;

            if (tmp > MAX_PATH_LEN)
                return FSE_NAMETOLONG;

            // Update curr inode to be the one we are currently in
            curr.d_inode = read_inode(ino_num);
            
            current_running->cwd = ino_num;
        }
        if (index == 0) // We are done parsing, and buf contains the last part of the path
        {
            ino_num = name2inode(buf, current_running->cwd);

            if (ino_num < 0)
                return FSE_DENOTFOUND;

            curr.d_inode = read_inode(ino_num);
            
            if(curr.d_inode.type == INTYPE_FILE)
                return FSE_DIRISFILE;
            // Change cwd one last time
            current_running->cwd = ino_num;
        }
        update_bitmaps();
        return FSE_OK;
    }
    else
    {
        // This is a relative path
        // Index = where to parse from next time
        index = parse_path(buf, path);
        // Tmp stores idx
        tmp = index;

        ino_num = name2inode(buf, current_running->cwd);

        while (index > 0) // While parsing
        {
            // Retrieve inode numer
            ino_num = name2inode(buf, current_running->cwd);

            // Error if it doesn't exist within directory
            if (ino_num < 0)
                return FSE_DENOTFOUND;

            index = parse_path(buf, &path[tmp]);
            tmp += index;
            if (tmp > MAX_PATH_LEN)
                return FSE_NAMETOLONG;

            curr.d_inode = read_inode(ino_num);
            current_running->cwd = ino_num;
        }
        if (index == 0) // Done parsing
        {
            ino_num = name2inode(buf, current_running->cwd);
            if (ino_num < 0)
                return FSE_DENOTFOUND;

            curr.d_inode = read_inode(ino_num);
            
            current_running->cwd = ino_num;
            
            // We are now done parsing, and should end up in a directory, not a file
            if(curr.d_inode.type == INTYPE_FILE)
                return FSE_DIRISFILE;

        }
        update_bitmaps();
        return FSE_OK;
    }
}

int fs_rmdir(char *path)
{
    return -1;
}

int fs_link(char *linkname, char *filename)
{
    char buf[MAX_FILENAME_LEN];
    struct disk_inode curr;
    int ino_num, index, tmp, start_cwd;


    if (filename[0] == '/')
    {
        // Abs path
        if (filename[1] == '\0')
        {
            // Can't create a link to a dir
            return FSE_INVALIDNAME;
        }

        // If DE with same name as linkname exists
        if (name2inode(linkname, current_running->cwd) > 0)
            return FSE_EXIST;
        
        // Read root inode
        curr = read_inode(0);
        start_cwd = current_running->cwd;
        ino_num = 0;

        index = parse_path(buf, &filename[1]);
        tmp = parse_path(buf, &filename[1]) + 1;

        // While there is more to parse
        while (index > 0)
        {
            ino_num = name2inode(buf, ino_num);

            if (ino_num < 0)
                return FSE_DENOTFOUND;

            index = parse_path(buf, &filename[tmp]);
            tmp += index;
            if (tmp > MAX_PATH_LEN)
                return FSE_NAMETOLONG;

            curr = read_inode(ino_num);
            current_running->cwd = ino_num;
        }
        // If we are done parsing
        if (index == 0)
        {
            // Get inode number of file we want to create a link to
            ino_num = name2inode(buf, current_running->cwd);

            // Return if it doesn't exist
            if (ino_num < 0)
                return FSE_DENOTFOUND;

            // Read in file
            struct disk_inode tmp_inode = read_inode(ino_num);
            
            // Make sure it is indeed a file
            if(tmp_inode.type == INTYPE_DIR)
                return FSE_INVALIDNAME;

            current_running->cwd = start_cwd;

            
            /* Create new dir entry for parent dir */
            struct dirent dir_entry;
            
            /* Allocate free inode in bitmap */
            dir_entry.inode = ino_num;
            if (dir_entry.inode < 0)
                return FSE_BITMAP;

            /* Copy linkname to dir entry name */
            strlcpy(dir_entry.name, linkname, sizeof(dir_entry.name));

            curr = read_inode(current_running->cwd);
            
            struct disk_inode new_entry = read_inode(dir_entry.inode);

            /* Size of disk inode of current dir */
            int size = curr.size;
            
            /* If inode size + size of new dir entry is too big, return error */
            if (size + sizeof(dir_entry) > d_super->max_filesize)
                    return FSE_ERROR;

            for (int i = 0; i < tmp_inode.size; i += BLOCK_SIZE)
                new_entry.direct[i] = tmp_inode.direct[i];

            
            tmp_inode.nlinks++;


            /* Write the dir entry to disk */
            block_modify(d_super->root_inode + idx2blk(curr.direct[size / BLOCK_SIZE]), size % BLOCK_SIZE,
                &dir_entry, sizeof(dir_entry));

            /* Change size of current dir */
            curr.size += sizeof(dir_entry);

            update_inode(curr, current_running->cwd);
            update_bitmaps();
            return FSE_OK;
        }

    }
    else
    {
        // Relpath
    }
    return FSE_OK;
}

int fs_unlink(char *linkname)
{

    return -1;
}


int fs_stat(int fd, char *buffer)
{
    struct disk_inode curr = read_inode(current_running->filedes[fd].idx);
    buffer[0] = curr.type;
    buffer[1] = curr.nlinks;
    bcopy((char *)&curr.size, &buffer[2], sizeof(int));
    return FSE_OK;
}

/* 
 * Helper functions for the system calls
 */

/*
 * get_free_entry:
 * 
 * Search the given bitmap for the first zero bit.  If an entry is
 * found it is set to one and the entry number is returned.  Returns
 * -1 if all entrys in the bitmap are set.
 */
static int get_free_entry(unsigned char *bitmap)
{
    int i;

    /* Seach for a free entry */
    for (i = 0; i < BITMAP_ENTRIES / 8; i++) {
    	if (bitmap[i] == 0xff)	/* All taken */
    	    continue;
    	if ((bitmap[i] & 0x80) == 0) {	/* msb */
    	    bitmap[i] |= 0x80;
    	    return i * 8;
    	} else if ((bitmap[i] & 0x40) == 0) {
    	    bitmap[i] |= 0x40;
    	    return i * 8 + 1;
    	} else if ((bitmap[i] & 0x20) == 0) {
    	    bitmap[i] |= 0x20;
    	    return i * 8 + 2;
    	} else if ((bitmap[i] & 0x10) == 0) {
    	    bitmap[i] |= 0x10;
    	    return i * 8 + 3;
    	} else if ((bitmap[i] & 0x08) == 0) {
    	    bitmap[i] |= 0x08;
    	    return i * 8 + 4;
    	} else if ((bitmap[i] & 0x04) == 0) {
    	    bitmap[i] |= 0x04;
    	    return i * 8 + 5;
    	} else if ((bitmap[i] & 0x02) == 0) {
    	    bitmap[i] |= 0x02;
    	    return i * 8 + 6;
    	} else if ((bitmap[i] & 0x01) == 0) {	/* lsb */
    	    bitmap[i] |= 0x01;
    	    return i * 8 + 7;
    	}
    }
    return -1;
}

/*
 * free_bitmap_entry:
 *
 * Free a bitmap entry, if the entry is not found -1 is returned, otherwise zero. 
 * Note that this function does not check if the bitmap entry was used (freeing
 * an unused entry has no effect).
 */
static int free_bitmap_entry(int entry, unsigned char *bitmap)
{
    unsigned char *bme;

    if (entry >= BITMAP_ENTRIES)
	   return -1;

    bme = &bitmap[entry / 8];

    switch (entry % 8) {
    case 0:
	   *bme &= ~0x80;
	   break;
    case 1:
	   *bme &= ~0x40;
	   break;
    case 2:
	   *bme &= ~0x20;
	   break;
    case 3:
	   *bme &= ~0x10;
	   break;
    case 4:
	   *bme &= ~0x08;
	   break;
    case 5:
	   *bme &= ~0x04;
	   break;
    case 6:
	   *bme &= ~0x02;
	   break;
    case 7:
	   *bme &= ~0x01;
	   break;
    }

    return 0;
}



/*
 * ino2blk:
 * Returns the filesystem block (block number relative to the super
 * block) corresponding to the inode number passed.
 */
static blknum_t ino2blk(inode_t ino)
{
    ino = (sizeof(struct disk_inode) * ino) / BLOCK_SIZE; 
    return ino;
}

/*
 * idx2blk:
 * Returns the filesystem block (block number relative to the super
 * block) corresponding to the data block index passed.
 */
static blknum_t idx2blk(int index)
{

   index = (index + INODE_BLOCKS);
   return index;
}

/*
 * name2inode:
 * Parses a file name and returns the corresponding inode number. If
 * the file cannot be found, -1 is returned.
 */
static inode_t name2inode(const char *name, inode_t num)
{
    int num_entries, j, i, start_block, offset;
    struct mem_inode tmp;
    if (name[0] == '/')
    {
        return FSE_NOTYETIMPLEMENTED;
    }
    else
    {
        /* Assume name is just a single path, not including '/' */
        tmp.d_inode = read_inode(num);

        /* How many entries are there in dir */
        num_entries = tmp.d_inode.size / sizeof(struct dirent);
        /* A dirent struct with enough space for inode and name of all entries */
        struct dirent dir[num_entries];

        // Reset pos
        tmp.pos = 0;

        for (i = 0; i < num_entries; i++) {

            // Where we want to start reading from
            start_block = d_super->root_inode + idx2blk(tmp.d_inode.direct[tmp.pos / BLOCK_SIZE]);

            offset = tmp.pos % BLOCK_SIZE;
            // We need to wrap if we get in here
            if (offset + sizeof(struct dirent) > BLOCK_SIZE)
            {
                // Bytes to read from first block
                j = BLOCK_SIZE - offset;
                // Read j bytes
                block_read_part(start_block, offset, j, &dir[i]);
                // Update pos
                tmp.pos += j;
                // Get new offset
                offset = tmp.pos % BLOCK_SIZE;
                // Get new start block
                start_block = d_super->root_inode + idx2blk(tmp.d_inode.direct[tmp.pos / BLOCK_SIZE]);
                // Read rest of the bytes
                block_read_part(start_block, offset, sizeof(struct dirent) - j, &((char *)&dir[i])[j]);
                // Update pos again
                tmp.pos += (sizeof(struct dirent) - j);
            }
            else
            {
                /* Read every dirent struct into the struct */
                block_read_part(start_block, offset, sizeof(struct dirent), &dir[i]);
                /* Update the pos to read from */
                tmp.pos += sizeof(struct dirent);
            }
            /* Return inode num if it exists */
            if (same_string((char *)name, dir[i].name))
            {
                tmp.pos = 0;
                return dir[i].inode;
            }
        }
        return FSE_DENOTFOUND;
    }
}


