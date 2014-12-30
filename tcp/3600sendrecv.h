/*
 * CS3600, Spring 2013
 * Project 4 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600SENDRECV_H__
#define __3600SENDRECV_H__

#include <stdio.h>
#include <stdarg.h>

#define MAX_WINDOW_SIZE 2000
#define TCP_PACKET_SIZE 1500
#define True 1
#define False 0

typedef struct header_t {
    unsigned int magic:14;
    unsigned int ack:1;
    unsigned int eof:1;
    unsigned short length;
    unsigned int sequence;
    unsigned int checksum;
} header;

unsigned int MAGIC;

unsigned short cksum(char * data, unsigned int data_length);
void dump_packet(unsigned char *data, int size);
header *make_header(int sequence, int length, int eof, int ack);
header *get_header(void *data);
char *get_data(void *data);
char *timestamp();
void mylog(char *fmt, ...);

#endif

