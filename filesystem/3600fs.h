/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600FS_H__
#define __3600FS_H__

// For non-conditionals
#define TRUE 1
#define FALSE 0

// Global Constants
#define END 0
#define MAX_FILES 100
#define MAGIC_NUMBER 481516236
#define MAX_FILENAME_LENGTH 28
#define DATA_BLOCKS_PER_FAT 128
#define MAX_SIZE_IN_BUF 4096

// VCB Struct
typedef struct vcb_s {

    // A magic number of identify your disk
    int magic;

    // Description of the disk layout
    int blocksize;
    int de_start;
    int de_length;
    int fat_start;
    int fat_length;
    int db_start;

    // Metadata for the root directory
    uid_t user;
    gid_t group;
    mode_t mode;

    struct timespec access_time;
    struct timespec modify_time;
    struct timespec create_time;

} vcb;

// Metadata for files
typedef struct dirent_s {

    unsigned int valid;
    unsigned int first_block;
    unsigned int size;

    uid_t user;
    gid_t group;
    mode_t mode;

    struct timespec access_time;
    struct timespec modify_time;
    struct timespec create_time;

    // Name of 27 characters and null terminating
    char name[MAX_FILENAME_LENGTH];

} dirent;

// Pointers to data
typedef struct fatent_s {

    unsigned int used:1;
    unsigned int eof:1;
    unsigned int next:30;

} fatent;

unsigned int get_block_start(unsigned int x);

#endif
