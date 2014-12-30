/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 */
#ifndef __3600DNS_H__
#define __3600DNS_H__

typedef struct answer {

    char *name;
    uint16_t atype;
    uint16_t aclass;
    uint32_t ttl;
    uint16_t rdlength;
    char *rdata;

} answer;

typedef struct packet {
    uint16_t id;
    unsigned int qr : 1;
    unsigned int opcode : 4;
    unsigned int aa : 1;
    unsigned int tc : 1;
    unsigned int rd : 1;
    unsigned int ra : 1;
    unsigned int z : 3;
    unsigned int rcode : 4;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
    char *qname;
    uint16_t qtype;
    uint16_t qclass;
    answer *packet_answers;

} packet;

const packet SETUP = {
    .id      = 1337,
    .qr      = 0,
    .opcode  = 0,
    .rd      = 1,
    .z       = 0,
    .rcode   = 0,
    .qdcount = 1,
    .ancount = 0,
    .nscount = 0,
    .arcount = 0,
    .qtype  = 0x0001,
    .qclass = 0x0001,
};

#endif

