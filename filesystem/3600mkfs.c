/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This program is intended to format your disk file, and should be executed
 * BEFORE any attempt is made to mount your file system.  It will not, however
 * be called before every mount (you will call it manually when you format
 * your disk file).
 */

#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "3600fs.h"
#include "disk.h"

void myformat(int size) {

    if (size < 103) {
        perror("Not enough blocks to format FAT disk");
    }

    // Do not touch or move this function
    dcreate_connect();

    // Setup VCB, block 0
    vcb v;
    v.magic = MAGIC_NUMBER;
    v.blocksize = BLOCKSIZE;

    // Block 1-101, 100 files total (not including 101);
    v.de_start = 1;
    v.de_length = 100;

    // Find the size for the FAT and data blocks
    int remaining_blocks = size - v.de_length - 1;
    int DATA_BLOCKS = (remaining_blocks * 128) / 129;
    int FAT_BLOCKS = remaining_blocks - DATA_BLOCKS;

    // Block 101-size FAT and data blocks
    v.fat_start = v.de_start + v.de_length;
    v.fat_length = FAT_BLOCKS;
    v.db_start = v.fat_start + v.fat_length;

    // Set metadata for VCB
    v.user = getuid();
    v.group = getgid();
    v.mode = 0777;

    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);

    v.access_time = current_time;
    v.create_time = current_time;
    v.modify_time = current_time;

    // Set metadata for DIRENTs
    dirent d;
    d.valid = FALSE;
    d.first_block = END;
    d.size = 0;
    d.user = getuid();
    d.group = getgid();
    d.mode = 0777;

    // Set metadata for FATs
    fatent f;
    f.used = FALSE;
    f.eof = TRUE;
    f.next = FALSE;

    // WRITE VCB, DIRENTs, FATs, and Data Blocks

    // Copy it to a BLOCKSIZE-d location
    char tmp[BLOCKSIZE];
    memset(tmp, 0, BLOCKSIZE);
    memcpy(tmp, &v, sizeof(vcb));

    // Finally, actually write it to disk in the 0th block
    if (dwrite(0, tmp) < 0)
        perror("Error writing to disk");

    // Do the same thing for dirents
    for(int i = v.de_start; i < (v.de_start + v.de_length); i++) {
        memset(tmp, 0, BLOCKSIZE);
        memcpy(tmp, &d, sizeof(dirent));
        if (dwrite(i, tmp) < 0)
            perror("Error writing to disk");
    }

    // Do the same thing for fatents
    for(int i = v.fat_start; i < (v.fat_start + v.fat_length); i++) {
        memset(tmp, 0, BLOCKSIZE);
        memcpy(tmp, &f, sizeof(fatent));
        if (dwrite(i, tmp) < 0)
            perror("Error writing to disk");
    }

    // Do the same thing for the data blocks
    for(int i = v.db_start; i < size; i++) {
        memset(tmp, 0, BLOCKSIZE);
        if (dwrite(i, tmp) < 0)
            perror("Error writing to disk");
    }

    // Do not touch or move this function
    dunconnect();
}

int main(int argc, char** argv) {

    // Do not touch this function
    if (argc != 2) {
        printf("Invalid number of arguments \n");
        printf("usage: %s diskSizeInBlockSize\n", argv[0]);
        return 1;
    }

    unsigned long size = atoi(argv[1]);
    printf("Formatting the disk with size %lu \n", size);
    myformat(size);
}
