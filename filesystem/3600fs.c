/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This file contains all of the basic functions that you will need
 * to implement for this project.  Please see the project handout
 * for more details on any particular function, and ask on Piazza if
 * you get stuck.
 */

#define FUSE_USE_VERSION 26

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#define _POSIX_C_SOURCE 199309

#include <time.h>
#include <fuse.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <sys/statfs.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "3600fs.h"
#include "disk.h"

vcb getvcb();
dirent getdirent(unsigned int i);
fatent getfatent(unsigned int i, unsigned int offset);
int savefatent(fatent fat, unsigned int i, unsigned int offset);
int savedirent(dirent dir, unsigned int i);

vcb our_vcb;

/*
 * Initialize filesystem. Read in file system metadata and initialize
 * memory structures. If there are inconsistencies, now would also be
 * a good time to deal with that.
 *
 * HINT: You don't need to deal with the 'conn' parameter AND you may
 * just return NULL.
 *
 */
static void* vfs_mount(struct fuse_conn_info *conn) {
    fprintf(stderr, "vfs_mount called\n");

    // Do not touch or move this code; connects the disk
    dconnect();

    /* 3600: YOU SHOULD ADD CODE HERE TO CHECK THE CONSISTENCY OF YOUR DISK
       AND LOAD ANY DATA STRUCTURES INTO MEMORY */

    // Global VCB is easier to handle
    our_vcb = getvcb();

    return NULL;
}

/*
 * Called when your file system is unmounted.
 *
 */
static void vfs_unmount (void *private_data) {
    fprintf(stderr, "vfs_unmount called\n");

    /* 3600: YOU SHOULD ADD CODE HERE TO MAKE SURE YOUR ON-DISK STRUCTURES
       ARE IN-SYNC BEFORE THE DISK IS UNMOUNTED (ONLY NECESSARY IF YOU
       KEEP DATA CACHED THAT'S NOT ON DISK */

    // Do not touch or move this code; unconnects the disk
    dunconnect();
}

/*
 *
 * Given an absolute path to a file/directory (i.e., /foo ---all
 * paths will start with the root directory of the CS3600 file
 * system, "/"), you need to return the file attributes that is
 * similar stat system call.
 *
 * HINT: You must implement stbuf->stmode, stbuf->st_size, and
 * stbuf->st_blocks correctly.
 *
 */
static int vfs_getattr(const char *path, struct stat *stbuf) {
    fprintf(stderr, "vfs_getattr called\n");

    // Do not mess with this code
    stbuf->st_nlink = 1; // hard links
    stbuf->st_rdev  = 0;
    stbuf->st_blksize = BLOCKSIZE;

    // Supports only one directory
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = 0777 | S_IFDIR;
        return 0;
    }

    for (int i = our_vcb.de_start; i < our_vcb.de_start + our_vcb.de_length; i++) {

        dirent d = getdirent(i);

        // Set stbuf to the metadata of the file
        if(strncmp(d.name, path + 1, MAX_FILENAME_LENGTH - 1) == 0) {
            stbuf->st_mode  = d.mode | S_IFREG;
            stbuf->st_uid = d.user;
            stbuf->st_gid = d.group;
            stbuf->st_atime = d.access_time.tv_sec;
            stbuf->st_mtime = d.modify_time.tv_sec;
            stbuf->st_ctime = d.create_time.tv_sec;
            stbuf->st_size = d.size;
            stbuf->st_blocks = d.size / BLOCKSIZE;

            return 0;
        }
    }

    // No such file or directory
    return -ENOENT;
}

/*
 * Given an absolute path to a directory (which may or may not end in
 * '/'), vfs_mkdir will create a new directory named dirname in that
 * directory, and will create it with the specified initial mode.
 *
 * HINT: Don't forget to create . and .. while creating a
 * directory.
 */
/*
 * NOTE: YOU CAN IGNORE THIS METHOD, UNLESS YOU ARE COMPLETING THE
 *       EXTRA CREDIT PORTION OF THE PROJECT.  IF SO, YOU SHOULD
 *       UN-COMMENT THIS METHOD.
 static int vfs_mkdir(const char *path, mode_t mode) {

 return -1;
 } */

/** Read directory
 *
 * Given an absolute path to a directory, vfs_readdir will return
 * all the files and directories in that directory.
 *
 * HINT:
 * Use the filler parameter to fill in, look at fusexmp.c to see an example
 * Prototype below
 *
 * Function to add an entry in a readdir() operation
 *
 * @param buf the buffer passed to the readdir() operation
 * @param name the file name of the directory entry
 * @param stat file attributes, can be NULL
 * @param off offset of the next entry or zero
 * @return 1 if buffer is full, zero otherwise
 * typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
 *                                 const struct stat *stbuf, off_t off);
 *
 * Your solution should not need to touch fi
 *
 */
static int vfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *fi)
{

    // If it's not in the one supported directory, return error
    if(path[0] != '/')
        return -1;

    dirent d;

    for(int i = our_vcb.de_start; i < our_vcb.de_start + our_vcb.de_length; i++) {

        d = getdirent(i);

        // If it is a valid file then add it to the filler
        if(d.valid == TRUE) {
            if (filler(buf, d.name, NULL, offset) != 0) {
                // There is no more space
                return -1;
            }
        }
    }

    // No error
    return 0;
}

/*
 * Given an absolute path to a file (for example /a/b/myFile), vfs_create
 * will create a new file named myFile in the /a/b directory.
 *
 */
static int vfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

    dirent d;
    int i = 0;
    for(i = our_vcb.de_start; i < our_vcb.de_start + our_vcb.de_length; i++) {
        d = getdirent(i);

        // Found a dirent that is not in use
        if(d.valid == FALSE) {
            break;
        }
    }

    // Adds the metadata to the dirent
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);

    memset(&d, 0, sizeof(dirent));
    d.valid = TRUE;
    d.first_block = END;
    d.size = 0;
    d.user = geteuid();
    d.group = getegid();
    d.access_time = time;
    d.modify_time = time;
    d.create_time = time;
    strncpy(d.name, path + 1, MAX_FILENAME_LENGTH);

    // Write the dirent to the filesystem
    savedirent(d, i);

    // No error
    return 0;
}

/*
 * The function vfs_read provides the ability to read data from
 * an absolute path 'path,' which should specify an existing file.
 * It will attempt to read 'size' bytes starting at the specified
 * offset (offset) from the specified file (path)
 * on your filesystem into the memory address 'buf'. The return
 * value is the amount of bytes actually read; if the file is
 * smaller than size, vfs_read will simply return the most amount
 * of bytes it could read.
 *
 * HINT: You should be able to ignore 'fi'
 *
 */
static int vfs_read(const char *path, char *buf, size_t size, off_t offset,
        struct fuse_file_info *fi)
{

    // Clear the incoming buf to zero
    memset(buf, 0, strlen(buf));

    // If the path is not supported, return error
    if(path[0] != '/')
        return -1;

    dirent d;

    int found = FALSE;
    for(unsigned int i = our_vcb.de_start; i < our_vcb.de_start + our_vcb.de_length; i++) {
        d = getdirent(i);

        // We found the file we want to read
        if(strncmp(d.name, path + 1, MAX_FILENAME_LENGTH - 1) == 0) {
            found = TRUE;
            break;
        }
    }

    // We are trying to read at an offset not supported or the
    // file was not found
    if(found == FALSE || offset > d.size)
        return -1;

    // Find the location of the first fatent
    int first_offset = d.first_block;
    int fat_block = first_offset / DATA_BLOCKS_PER_FAT;
    int fat_offset = first_offset % DATA_BLOCKS_PER_FAT;

    // Load the first fatent
    fatent f = getfatent(our_vcb.fat_start + fat_block, fat_offset);

    // Keep track of the amount of bytes left to read and read. Buf can only hold up to
    // 4096 bytes at a time.
    int bytes_left_to_read = size;
    int bytes_read = 0;

    // Start reading at this fatent
    int offset_block = offset / BLOCKSIZE;

    int iteration = 0;
    while(f.eof != TRUE) {
        if(iteration == offset_block) {

            // We found the offset block
            break;
        }

        // Get the data for the next fatent
        first_offset = f.next;
        fat_block = first_offset / DATA_BLOCKS_PER_FAT;
        fat_offset = first_offset % DATA_BLOCKS_PER_FAT;

        // Load the fatent
        f = getfatent(fat_block + our_vcb.fat_start, fat_offset);
        iteration++;
    }

    // Load the first fatent's datablock and put it into buf
    char block[BLOCKSIZE];
    memset(block, 0, BLOCKSIZE);
    dread(our_vcb.db_start + d.first_block, block);
    memcpy(&buf[bytes_read], block, BLOCKSIZE);
    bytes_read += BLOCKSIZE;
    bytes_left_to_read -= BLOCKSIZE;

    while(f.eof != TRUE) {

        // If we have maxed out the buffer then return or there's nothing left
        // to read
        if(bytes_read == MAX_SIZE_IN_BUF || bytes_left_to_read < 0) {
            break;
        }

        // Get the location for the next fatent
        first_offset = f.next;
        fat_block = first_offset / DATA_BLOCKS_PER_FAT;
        fat_offset = first_offset % DATA_BLOCKS_PER_FAT;

        // Add the data from the fatent
        char block[BLOCKSIZE];
        memset(block, 0, BLOCKSIZE);
        dread(our_vcb.db_start + first_offset, block);
        memcpy(&buf[bytes_read], block, BLOCKSIZE);

        // Increment the reads done and left
        bytes_read += BLOCKSIZE;
        bytes_left_to_read -= BLOCKSIZE;

        // Load the new fatent
        f = getfatent(fat_block + our_vcb.fat_start, fat_offset);

    }

    // Return the number of bytes read. Should not be greater than the size of
    // the buffer
    return bytes_read;

}

/*
 * The function vfs_write will attempt to write 'size' bytes from
 * memory address 'buf' into a file specified by an absolute 'path'.
 * It should do so starting at the specified offset 'offset'.  If
 * offset is beyond the current size of the file, you should pad the
 * file with 0s until you reach the appropriate length.
 *
 * You should return the number of bytes written.
 *
 * HINT: Ignore 'fi'
 */
static int vfs_write(const char *path, const char *buf, size_t size,
        off_t offset, struct fuse_file_info *fi)
{

    // If the path is not supported, return error
    if(path[0] != '/')
        return -1;

    // If we are trying to write 0 bytes, return
    if(size == 0)
        return 0;

    dirent d;

    int found_location = 0;
    int found = FALSE;
    for(unsigned int i = our_vcb.de_start; i < our_vcb.de_start + our_vcb.de_length; i++) {
        d = getdirent(i);

        // We found the file we want to write to
        if(strncmp(d.name, path + 1, MAX_FILENAME_LENGTH - 1) == 0) {
            found = TRUE;
            found_location = i;
            break;
        }
    }

    // If the file was not found then return error
    if(found == FALSE)
        return -1;

    // Update the dirent
    d.valid = TRUE;
    d.size = size + offset;
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    d.access_time = current_time;
    d.modify_time = current_time;
    savedirent(d, found_location);

    // Get the offset start block
    int fatent_start_block = offset / BLOCKSIZE / DATA_BLOCKS_PER_FAT;
    int fatent_start_offset = (offset / BLOCKSIZE) % DATA_BLOCKS_PER_FAT;
    int buf_counter = 0;
    int total_size = size;

    // Find a free fatent
    for(unsigned int i = our_vcb.fat_start; i < our_vcb.fat_start + our_vcb.fat_length; i++) {
        for(unsigned int j = 0; j < DATA_BLOCKS_PER_FAT; j++) {

            fatent f = getfatent(i, j);
            unsigned int data_block = ((i - our_vcb.fat_start) * DATA_BLOCKS_PER_FAT) + j;

            // If dirent doesn't have a first_block
            if (f.used == FALSE && d.first_block == END) {

                d.first_block = data_block;
                savedirent(d, found_location);

                // Setup and write first fatent
                f.used = TRUE;
                f.eof = TRUE;
                f.next = FALSE;

                savefatent(f, i, j);

                // Setup and write data block
                char block [BLOCKSIZE];
                memset(block, 0, BLOCKSIZE);
                memcpy(block, &buf[buf_counter], BLOCKSIZE);

                dwrite(our_vcb.db_start + data_block, block);

                // Increment the reads done and left
                buf_counter += BLOCKSIZE;
                total_size -= BLOCKSIZE;

                if(total_size < 0) {
                    return size;
                }

            } else if (f.used == FALSE) {

                int first_offset = d.first_block;
                int fat_block = first_offset / DATA_BLOCKS_PER_FAT;
                int fat_offset = first_offset % DATA_BLOCKS_PER_FAT;

                fatent next = getfatent(our_vcb.fat_start + fat_block, fat_offset);

                while(next.eof != TRUE) {

                    first_offset = next.next;
                    fat_block = first_offset / DATA_BLOCKS_PER_FAT;
                    fat_offset = first_offset % DATA_BLOCKS_PER_FAT;

                    next = getfatent(fat_block + our_vcb.fat_start, fat_offset);
                }

                // Save the next to last fatent
                next.used = TRUE;
                next.eof = FALSE;
                next.next = data_block;
                savefatent(next, fat_block + our_vcb.fat_start, fat_offset);

                // Setup and write last fatent
                f.used = TRUE;
                f.eof = TRUE;
                f.next = FALSE;
                savefatent(f, i, j);

                // Setup and write data block
                char block [BLOCKSIZE];
                memset(block, 0, BLOCKSIZE);
                memcpy(block, &buf[buf_counter], BLOCKSIZE);
                dwrite(our_vcb.db_start + data_block, block);

                // Increment the reads done and left
                buf_counter += BLOCKSIZE;
                total_size -= BLOCKSIZE;

                if(total_size < 0) {
                    return size;
                }
            }
        }
    }

    return size;
}

/**
 * This function deletes the last component of the path (e.g., /a/b/c you
 * need to remove the file 'c' from the directory /a/b).
 */
static int vfs_delete(const char *path)
{

    /* 3600: NOTE THAT THE BLOCKS CORRESPONDING TO THE FILE SHOULD BE MARKED
       AS FREE, AND YOU SHOULD MAKE THEM AVAILABLE TO BE USED WITH OTHER FILES */

    dirent d;

    int found = FALSE;
    int i = 0;
    for(i = our_vcb.de_start; i < our_vcb.de_start + our_vcb.de_length; i++) {

        d = getdirent(i);

        // Found the dirent to delete
        if (strncmp(d.name, path + 1, MAX_FILENAME_LENGTH - 1) == 0) {
            found = TRUE;
            break;
        }
    }

    // We didn't find the dirent, return error
    if(found == FALSE)
        return -1;

    // If the first block is not empty, find the associated fatents and wipe
    // them
    if (d.first_block != END) {
        int first_offset = d.first_block;
        int fat_block = first_offset / DATA_BLOCKS_PER_FAT;
        int fat_offset = first_offset % DATA_BLOCKS_PER_FAT;

        fatent next = getfatent(our_vcb.fat_start + fat_block, fat_offset);

        fatent empty;
        memset(&empty, 0, sizeof(fatent));

        // Setup empty fatent data
        empty.used = FALSE;
        empty.eof = TRUE;
        empty.next = FALSE;

        // Remove the fatents until eof
        while(next.eof != TRUE) {
            first_offset = next.next;
            fat_block = first_offset / DATA_BLOCKS_PER_FAT;
            fat_offset = first_offset % DATA_BLOCKS_PER_FAT;

            // Get the next fatent to wipe
            next = getfatent(our_vcb.fat_start + fat_block, fat_offset);

            // Write an empty fatent
            savefatent(empty, our_vcb.fat_start + fat_block, fat_offset);
        }
    }

    // Reset the dirent
    memset(&d, 0, sizeof(dirent));
    d.valid = FALSE;
    d.first_block = END;
    d.size = BLOCKSIZE;
    savedirent(d, i);

    // No error
    return 0;
}

/*
 * The function rename will rename a file or directory named by the
 * string 'oldpath' and rename it to the file name specified by 'newpath'.
 *
 * HINT: Renaming could also be moving in disguise
 *
 */
static int vfs_rename(const char *from, const char *to)
{

    dirent d;
    int i = 0;
    for(i = our_vcb.de_start; i < our_vcb.de_start + our_vcb.de_length; i++) {
        d = getdirent(i);

        if(strncmp(d.name, from + 1, MAX_FILENAME_LENGTH -1) == 0) {

            // Found the dirent or file to rename
            strncpy(d.name, to, MAX_FILENAME_LENGTH);
            d.name[27] = '\0';
            break;
        }
    }

    // Write the new metadata
    savedirent(d, i);

    // No error
    return 0;

}


/*
 * This function will change the permissions on the file
 * to be mode.  This should only update the file's mode.
 * Only the permission bits of mode should be examined
 * (basically, the last 16 bits).  You should do something like
 *
 * fcb->mode = (mode & 0x0000ffff);
 *
 */
static int vfs_chmod(const char *file, mode_t mode)
{

    dirent d;
    int i = 0;
    for(i = our_vcb.de_start; i < our_vcb.de_start + our_vcb.de_length; i++) {
        d = getdirent(i);

        if(strncmp(d.name, file + 1, MAX_FILENAME_LENGTH -1) == 0) {

            // Found the dirent or file to change the mode permissions
            d.mode = mode;
            break;
        }
    }

    // Write the new metadata
    savedirent(d, i);

    // No error
    return 0;
}

/*
 * This function will change the user and group of the file
 * to be uid and gid.  This should only update the file's owner
 * and group.
 */
static int vfs_chown(const char *file, uid_t uid, gid_t gid)
{

    dirent d;
    int i = 0;
    for(i = our_vcb.de_start; i < our_vcb.de_start + our_vcb.de_length; i++) {
        d = getdirent(i);

        if(strncmp(d.name, file + 1, MAX_FILENAME_LENGTH -1) == 0) {

            // Found the dirent or file to change the uid and gid
            d.user = uid;
            d.group = gid;
            break;
        }
    }

    // Write the new metadata
    savedirent(d, i);

    // No error
    return 0;
}

/*
 * This function will update the file's last accessed time to
 * be ts[0] and will update the file's last modified time to be ts[1].
 */
static int vfs_utimens(const char *file, const struct timespec ts[2])
{

    dirent d;
    int i = 0;
    for(i = our_vcb.de_start; i < our_vcb.de_start + our_vcb.de_length; i++) {
        d = getdirent(i);

        if(strncmp(d.name, file + 1, MAX_FILENAME_LENGTH -1) == 0) {

            // Found the dirent or file to change the last access time and
            // modified time
            d.access_time = ts[0];
            d.create_time = ts[1];
            break;
        }
    }

    // Write the new metadata
    savedirent(d, i);

    // No error
    return 0;
}

/*
 * This function will truncate the file at the given offset
 * (essentially, it should shorten the file to only be offset
 * bytes long).
 */
static int vfs_truncate(const char *file, off_t offset)
{

    /* 3600: NOTE THAT ANY BLOCKS FREED BY THIS OPERATION SHOULD
       BE AVAILABLE FOR OTHER FILES TO USE. */

    // If the file is not part of the supported directory, return error
    if(file[0] != '/')
        return -1;

    dirent d;

    int found_location = 0;
    int found = FALSE;
    for(unsigned int i = our_vcb.de_start; i < our_vcb.de_start + our_vcb.de_length; i++) {
        d = getdirent(i);

        if(strncmp(d.name, file + 1, MAX_FILENAME_LENGTH - 1) == 0) {

            // We found the file we want to truncate
            found = TRUE;
            found_location = i;
            break;
        }
    }

    // If we didn't find the file or you are trying to truncate at too great of
    // an offset
    if(found == FALSE || offset > d.size)
        return -1;

    // Find the offset block or the endblock
    int offset_block = offset / BLOCKSIZE;
    int end_block = d.size / BLOCKSIZE;

    // Update the dirent
    d.valid = TRUE;
    d.size = offset;
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    d.access_time = current_time;
    d.modify_time = current_time;
    savedirent(d, found_location);

    // If we already reached the endblock return
    if (offset_block == end_block)
        return 0;

    // Get the first fatent data
    int first_offset = d.first_block;
    int fat_block = first_offset / DATA_BLOCKS_PER_FAT;
    int fat_offset = first_offset % DATA_BLOCKS_PER_FAT;

    // Load fatent
    fatent f = getfatent(our_vcb.fat_start + fat_block, fat_offset);

    int iteration = 0;
    while(f.eof != TRUE) {
        if(iteration == offset_block) {

            // Our fatent has reached the endblock
            break;
        }

        // Get the next fatent data
        first_offset = f.next;
        fat_block = first_offset / DATA_BLOCKS_PER_FAT;
        fat_offset = first_offset % DATA_BLOCKS_PER_FAT;

        // Load the next fatent
        f = getfatent(fat_block + our_vcb.fat_start, fat_offset);
        iteration++;
    }

    // We reached eof. We're trying to truncate at too high of an offset
    if (f.eof == TRUE)
        return -1;

    // Load the endblock fatent
    fatent next = getfatent(f.next / DATA_BLOCKS_PER_FAT, f.next % DATA_BLOCKS_PER_FAT);

    // Set the metadata to be the end
    f.eof = TRUE;
    f.next = END;
    f.used = TRUE;
    savefatent(f, our_vcb.fat_start + fat_block, fat_offset);

    // Keep going and reset the trailing fatents
    while(next.eof != TRUE) {

        first_offset = next.next;

        // Reset the fatent metadata
        next.used = FALSE;
        next.eof = TRUE;
        next.next = END;

        // Save it
        savefatent(next, our_vcb.fat_start + fat_block, fat_offset);

        // Get the next fatent
        fat_block = first_offset / DATA_BLOCKS_PER_FAT;
        fat_offset = first_offset % DATA_BLOCKS_PER_FAT;

        // Load it
        next = getfatent(fat_block + our_vcb.fat_start, fat_offset);
    }

    // Reset the very last fatent metadata
    next.used = FALSE;
    next.eof = TRUE;
    next.next = END;

    // Save it as well
    savefatent(next, our_vcb.fat_start + fat_block, fat_offset);

    // No errors
    return 0;

}


/*
 * You shouldn't mess with this; it sets up FUSE
 *
 * NOTE: If you're supporting multiple directories for extra credit,
 * you should add
 *
 *     .mkdir	 = vfs_mkdir,
 */
static struct fuse_operations vfs_oper = {
    .init    = vfs_mount,
    .destroy = vfs_unmount,
    .getattr = vfs_getattr,
    .readdir = vfs_readdir,
    .create	 = vfs_create,
    .read	 = vfs_read,
    .write	 = vfs_write,
    .unlink	 = vfs_delete,
    .rename	 = vfs_rename,
    .chmod	 = vfs_chmod,
    .chown	 = vfs_chown,
    .utimens	 = vfs_utimens,
    .truncate	 = vfs_truncate,
};

int main(int argc, char *argv[]) {
    /* Do not modify this function */
    umask(0);
    if ((argc < 4) || (strcmp("-s", argv[1])) || (strcmp("-d", argv[2]))) {
        printf("Usage: ./3600fs -s -d <dir>\n");
        exit(-1);
    }
    return fuse_main(argc, argv, &vfs_oper, NULL);
}

// Gets the vcb
vcb getvcb() {

    char block [BLOCKSIZE];
    memset(block, 0, BLOCKSIZE);
    if (dread(0, block) < 0)
        perror("Error reading from disk");

    vcb v;
    memcpy(&v, block, sizeof(vcb));
    return v;

}

// Gets the dirent at block i
dirent getdirent(unsigned int i) {

    char block [BLOCKSIZE];
    memset(block, 0, BLOCKSIZE);
    if (dread(i, block) < 0)
        perror("Error reading from disk");

    dirent d;
    memcpy(&d, block, sizeof(dirent));
    return d;
}

// Gets the fatent at block i and at offset
fatent getfatent(unsigned int i, unsigned int offset) {

    char block [BLOCKSIZE];
    memset(block, 0, BLOCKSIZE);
    if (dread(i, block) < 0)
        perror("Error reading from disk");

    fatent f;
    memcpy(&f, block + (offset * sizeof(fatent)), sizeof(fatent));
    return f;
}

// Save the dirent
int savedirent(dirent dir, unsigned int i) {

    char block [BLOCKSIZE];
    memset(block, 0, BLOCKSIZE);
    memcpy(block, &dir, sizeof(dirent));

    if(dwrite(i, block) < 0) {
        perror("Error writing to disk");
        return -1;
    }
}

// Save the fatent
int savefatent(fatent fat, unsigned int i, unsigned int offset) {

    char block [BLOCKSIZE];
    memset(block, 0, BLOCKSIZE);
    if (dread(i, block) < 0)
        perror("Error reading from disk");

    memcpy(block + (offset * sizeof(fatent)), &fat, sizeof(fatent));
    if(dwrite(i, block) < 0) {
        perror("Error writing to disk");
        return -1;
    }
}
