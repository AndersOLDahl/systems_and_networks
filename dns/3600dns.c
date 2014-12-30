/*
 * CS3600, Spring 2014
 * Project 3 Starter Code
 * (c) 2013 Alan Mislove
 */

#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "3600dns.h"

void push_to_packet(void ** packet, void * offset, size_t size);
void pull_from_packet(void ** packet, void * offset, size_t size);
char * read_name(void ** packet, void * offset);
size_t size_of_packet(packet input);
char *get_offset(void * packet, int offset);
answer *read_answers(void ** packet, void * offset, size_t count);
answer *set_answer(void **ptr, void *packet);
packet read_response(unsigned char *response);
void get_ip(char *rdata);
void create_packet_to_send(unsigned char **output, packet input);
void print_response(answer input);
static void dump_packet(unsigned char *data, int size);

// Pushes data to the packet
void push_to_packet(void ** packet, void * offset, size_t size) {
    memcpy(*packet, offset, size);
    *packet = size + *packet;
}

// Pull data from the packet
void pull_from_packet(void ** packet, void * offset, size_t size) {
    memcpy(offset, *packet, size);
    *packet = size + *packet;
}

// Function to return the size of a packet (8 uint16_t's). Added one for the zero length octet.
size_t size_of_packet(packet input) {
    return sizeof(uint16_t) * 8 + strlen(input.qname) + 1;
}

// Get the offset from the packet
char * get_name_start(void * packet, int offset) {
    void *offset_packet = packet + offset;
    char *offset_name = read_name(&offset_packet, packet);
    return offset_name;
}

// Read the name from the packet
char * read_name(void ** packet, void * offset) {
    uint16_t header = ntohs(*(uint16_t *) *packet);
    char * name = calloc(sizeof(char), 1024);
    char * c = name;

    // If the first two bits are set, then we know it's a DNS
    // pointer.
    if ((header & 0xC000) == 0xC000) {
        *packet += 2;
        int off = header & 0x3FFF;
        strcpy(c, get_name_start(offset, off));

        // Otherwise it is a number that specifies a string
    } else {

        int length = 0;
        pull_from_packet(packet, &length, sizeof(char));

        int i = 0;
        while(i < length) {
            pull_from_packet(packet, c++, sizeof(char));
            i++;
        }

        if (*(char *) *packet == 0) {
            pull_from_packet(packet, c, sizeof(char));

        } else {

            // Add the missing period
            char end = '.';
            memcpy(c++, &end, sizeof(char));

            // Get the next word of the name
            char * next = read_name(packet, offset);
            strcpy(c, next);

            // FREE malloced variable
            free(next);
        }
    }

    // Returning malloced variable
    return name;
}

// Read all of the answers from the response
answer *read_answers(void ** packet, void * offset, size_t count) {
    answer *answers;

    // We do not have any answers to count
    if (count <= 0) {
        answers = NULL;

        // We do have answers to read
    } else {
        answers = malloc(count * sizeof(answer));

        for (int i = 0; i < (int) count; i++) {
            answer *item = set_answer(packet, offset);
            answers[i] = *item;
        }
    }

    return answers;
}

// Set the answers
answer *set_answer(void **packet, void *offset) {
    answer *response;
    response = malloc(sizeof(answer));

    // Set the name
    response->name = read_name(packet, offset);

    // Set the atype
    uint16_t atype;
    pull_from_packet(packet, &atype, sizeof(uint16_t));
    response->atype = ntohs(atype);

    // Set the aclass
    uint16_t aclass;
    pull_from_packet(packet, &aclass, sizeof(uint16_t));
    response->aclass = ntohs(aclass);

    // Set the rdlength
    uint16_t rdlength;
    pull_from_packet(packet, &rdlength, sizeof(uint16_t));
    response->rdlength = ntohs(rdlength);

    // Set the ttl
    uint32_t ttl;
    pull_from_packet(packet, &ttl, sizeof(uint32_t));
    response->ttl = ntohl(ttl);

    if (response->atype == 0x0001) {
        uint32_t *rdata = malloc(sizeof(uint32_t));
        pull_from_packet(packet, rdata, sizeof(uint32_t));
        *rdata = ntohl(*rdata);
        response->rdata = *rdata;
    } else if (response->atype == 0x0005) {
        response->rdata = read_name(packet, offset);
    }

    return response;
}

// Read the response from the packet
packet read_response(unsigned char *response) {
    packet *returning = calloc(sizeof(packet), 1);
    void *ptr_to_response = response;
    uint16_t id = 0;
    uint16_t flags;

    // Fetch the id
    pull_from_packet(&ptr_to_response, &id, sizeof(uint16_t));
    returning->id = ntohs(id);

    // Fetch the flags
    pull_from_packet(&ptr_to_response, &flags, sizeof(uint16_t));
    flags = ntohs(flags);
    returning->qr = (0x1 & (flags >> 15));
    returning->opcode = (0xF & (flags >> 11));
    returning->aa = (0x1 & (flags >> 10));
    returning->tc = (0x1 & (flags >> 9));
    returning->rd = (0x1 & (flags >> 8));
    returning->ra = (0x1 & (flags >> 7));
    returning->z = (0x1 & (flags >> 4));
    returning->rcode = (0xF & flags);

    // Fetch qdcount
    uint16_t qdcount;
    pull_from_packet(&ptr_to_response, &qdcount, sizeof(uint16_t));
    returning->qdcount = ntohs(qdcount);

    // Fetch ancount
    uint16_t ancount;
    pull_from_packet(&ptr_to_response, &ancount, sizeof(uint16_t));
    returning->ancount = ntohs(ancount);

    // Fetch nscount
    uint16_t nscount;
    pull_from_packet(&ptr_to_response, &nscount, sizeof(uint16_t));
    returning->nscount = ntohs(nscount);

    // Fetch arcount
    uint16_t arcount;
    pull_from_packet(&ptr_to_response, &arcount, sizeof(uint16_t));
    returning->arcount = ntohs(arcount);

    // Fetch qname
    returning->qname = read_name(&ptr_to_response, &response); //derz, write!

    // Fetch qtype
    uint16_t qtype;
    pull_from_packet(&ptr_to_response, &qtype, sizeof(uint16_t));
    returning->qtype = ntohs(qtype);

    // Fetch qclass
    uint16_t qclass;
    pull_from_packet(&ptr_to_response, &qclass, sizeof(uint16_t));
    returning->qclass = ntohs(qclass);

    // Fetch packet answers
    returning->packet_answers = read_answers(&ptr_to_response, response, returning->ancount);

    // Return the packet
    return *returning;
}

// Creates the packet based on the input struct
void create_packet_to_send(unsigned char **output, packet input) {

    // Malloc enough space for the packet
    *output = malloc(size_of_packet(input));
    void *ptr_to_output = *output;

    // Our common push size for most fields
    size_t push_size = sizeof(uint16_t);

    // Convert the id to network byte order
    uint16_t id = htons(input.id);
    push_to_packet(&ptr_to_output, &id, push_size);

    // Setup the bits for the proper encoding. Adds the bits that are enabled or
    // set to 1.
    uint16_t flags = ((input.qr << 15)     & 0x8000) |
        ((input.opcode << 11) & 0x7800) |
        ((input.aa << 10)     & 0x400)  |
        ((input.tc << 9)      & 0x200)  |
        ((input.rd << 8)      & 0x100)  |
        ((input.ra << 7)      & 0x80)   |
        ((input.z << 4)       & 0x70)   |
        (input.rcode         & 0xF);

    // Convert the flags to network byte order and push the flags
    flags = htons(flags);
    push_to_packet(&ptr_to_output, &flags, push_size);

    // Count the number of questions and convert it to network byte order
    uint16_t qdcount = htons(input.qdcount);
    push_to_packet(&ptr_to_output, &qdcount, push_size);

    // Count of the answers and convert it to network byte order
    uint16_t ancount = htons(input.ancount);
    push_to_packet(&ptr_to_output, &ancount, push_size);

    // No records follow, use the default
    uint16_t nscount = htons(input.nscount);
    push_to_packet(&ptr_to_output, &nscount, push_size);

    // No additional answers follow, use the default
    uint16_t arcount = htons(input.arcount);
    push_to_packet(&ptr_to_output, &arcount, push_size);

    // Push the name we are looking for in network byte order
    push_to_packet(&ptr_to_output, input.qname, strlen(input.qname) + 1);

    // Push the type of query in network byte order
    uint16_t qtype = htons(input.qtype);
    push_to_packet(&ptr_to_output, &qtype, push_size);

    // Push the type of query in network type order
    uint16_t qclass = htons(input.qclass);
    push_to_packet(&ptr_to_output, &qclass, push_size);
}

void get_ip(char *rdata) {

    // Print the ip
    printf("%d.%d.%d.%d", ((int) rdata >> 24) & 0xFF, ((int)rdata >> 16) & 0xFF, ((int)rdata >> 8) & 0xFF, (int) rdata & 0xFF);
}


void print_response(answer input) {

    // Print the response
    uint16_t atype = input.atype;
    if(atype == 0x0001) {
        printf("IP\t");
        get_ip(input.rdata);
    } else if (atype == 0x0005) {
        printf("CNAME\t%s", input.rdata);
    }
}

/**
 * This function will print a hex dump of the provided packet to the screen
 * to help facilitate debugging.  In your milestone and final submission, you
 * MUST call dump_packet() with your packet right before calling sendto().
 * You're welcome to use it at other times to help debug, but please comment those
 * out in your submissions.
 *
 * DO NOT MODIFY THIS FUNCTION
 *
 * data - The pointer to your packet buffer
 * size - The length of your packet
 */
static void dump_packet(unsigned char *data, int size) {
    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {

            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
                    ((unsigned int)p-(unsigned int)data) );
        }

        c = *p;
        if (isprint(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) {
            /* line completed */
            printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
    }
}

int main(int argc, char *argv[]) {
    /**
     * I've included some basic code for opening a socket in C, sending
     * a UDP packet, and then receiving a response (or timeout).  You'll
     * need to fill in many of the details, but this should be enough to
     * get you started.
     */

    // process the arguments
    if (argc != 3) {
        exit(1);
    }

    char *ip;
    int port = 53;
    if (argv[1][0] == '@') {
        char *temp = strdup(argv[argc-2] + 1);
        char *token;

        int counter = 0;
        while ((token = strsep(&temp, ":")) != NULL) {
            if (counter == 0) {
                ip = token;
            } else if (counter == 1) {
                port = (int) strtol(token, NULL, 10);
            } else {
                exit(1);
            }
            counter++;
        }
        free(temp);
    } else {
        exit(1);
    }

    // Construct the DNS request using our default as the base
    packet input = SETUP;

    // Duplicate the name
    char *name = strdup(argv[argc-1]);

    // Two extra for the leading count and NULL byte
    input.qname = malloc(strlen(name) + 2);

    // Add the name and the lengths in front of them
    void *ptr2 = input.qname;
    char *token;
    while ((token = strsep(&name, ".")) != NULL) {
        char length = strlen(token);
        push_to_packet(&ptr2, &length, 1);
        push_to_packet(&ptr2, token, length);
    }

    // Push the last byte after the char
    char byte = 0x00;
    push_to_packet(&ptr2, &byte, sizeof(char));

    // Free name
    free(name);

    // Change the input type if it changed
    input.qtype = 0x0001;

    // Send the DNS request and dump the packet with your request
    unsigned char *packet;
    size_t packet_len = size_of_packet(input);
    create_packet_to_send(&packet, input);

    // Open the UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Next, construct the destination address
    struct sockaddr_in out;
    out.sin_family = AF_INET;
    out.sin_port = htons(port);
    out.sin_addr.s_addr = inet_addr(ip);

    dump_packet(packet, packet_len);
    if (sendto(sock, packet, packet_len, 0, &out, sizeof(out)) < 0) {
        printf("There has been an error creating the socket! Exiting!");
        exit(1);
    }

    // Wait for the DNS reply (timeout: 5 seconds)
    struct sockaddr_in in;
    socklen_t in_len;

    // construct the socket set
    fd_set socks;
    FD_ZERO(&socks);
    FD_SET(sock, &socks);

    // construct the timeout
    struct timeval t;
    t.tv_sec = 5;
    t.tv_usec = 0;

    size_t size_of_buffer = 65536;
    unsigned char *buffer = calloc(size_of_buffer, sizeof(unsigned char));

    // wait to receive, or for a timeout*/
    if (select(sock + 1, &socks, NULL, NULL, &t)) {
        if (recvfrom(sock, buffer, size_of_buffer, 0, &in, &in_len) < 0) {
            printf("There has been an error reading the socket! Exiting!");
            exit(1);
        }
    } else {
        printf("NORESPONSE\n");
        exit(1);
    }

    // Read the reponse from the buffer
    struct packet response = read_response(buffer);

    // Process the response
    if (response.id != input.id) {
        printf("This isn't our packet! Exiting!");
        exit(1);
    } else if (response.opcode != 0 || response.rd != 1 || response.qr != 1 ||  response.qdcount != 1 || response.qtype != input.qtype || response.qclass != input.qclass) {
        printf("There has been an error in the response packet! Exiting!");
        exit(1);
    } else if (response.ancount == 0) {
        printf("NOTFOUND\n");
    }

    // Print all of the answers from the DNS server
    int count = response.ancount;
    for (int i=0; i < count; i++) {
        answer a = response.packet_answers[i];
        print_response(a);
        if (response.aa) {
            printf("\tauth\n");
        } else {
            printf("\tnonauth\n");
        }
    }

    // Return no error
    return 0;
}
