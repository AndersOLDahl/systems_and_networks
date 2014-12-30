/*
 * CS3600, Spring 2013
 * Project 4 Starter Code
 * (c) 2013 Alan Mislove
 *
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

#include "3600sendrecv.h"

static int DATA_SIZE = 1460;

unsigned int sequence = 1;

int main(int argc, char *argv[]);
void usage();
int get_next_data(char *data, int size);
void *get_next_packet(int sequence, int *len);
void send_packets(int sock, struct sockaddr_in out);
void send_final_packet(int sock, struct sockaddr_in out);
int load_packets_into_window(int done);
void receive_acknowledgements(int sock, fd_set socks, struct timeval t, struct sockaddr_in in, socklen_t in_len);

// Create the window and length for each packet
void * window[MAX_WINDOW_SIZE];
int packet_lengths[MAX_WINDOW_SIZE];

// Start with sequence 1. It is starts off as unacknowledged
unsigned int last_acked_packet = 0;

void usage() {
    printf("Usage: 3600send host:port\n");
    exit(1);
}

/**
 * Reads the next block of data from stdin
 */
int get_next_data(char *data, int size) {
    return read(0, data, size);
}

/**
 * Builds and returns the next packet, or NULL
 * if no more data is available.
 */
void *get_next_packet(int sequence, int *len) {
    char *data = malloc(DATA_SIZE);
    int data_len = get_next_data(data, DATA_SIZE);

    if (data_len == 0) {
        free(data);
        return NULL;
    }

    header *myheader = make_header(sequence, data_len, 0, 0);
    void *packet = malloc(sizeof(header) + data_len);
    memcpy(packet, myheader, sizeof(header));
    memcpy(((char *) packet) +sizeof(header), data, data_len);

    free(data);
    free(myheader);

    *len = sizeof(header) + data_len;

    return packet;
}

// Send the non-acked packets in the window.
void send_packets(int sock, struct sockaddr_in out) {

    // Send the packets in our window that still haven't been acknowledged.
    // 1 to 2001
    for (unsigned int i = last_acked_packet; i < sequence; i++) {

        // Send them
        if (sendto(sock, window[i % MAX_WINDOW_SIZE], packet_lengths[i % MAX_WINDOW_SIZE], 0, (struct sockaddr *) &out, (socklen_t) sizeof(out)) < 0) {
            perror("sendto");
            exit(1);
        }

        mylog("[send data] %d (%d)\n", i, packet_lengths[i % MAX_WINDOW_SIZE] - sizeof(header));
    }
}

void send_final_packet(int sock, struct sockaddr_in out) {

    for (unsigned int i = 0; i < 5; i++) {

        header *myheader = make_header(sequence, 0, 1, 0);

        if (sendto(sock, myheader, sizeof(header), 0, (struct sockaddr *) &out, (socklen_t) sizeof(out)) < 0) {
            perror("sendto");
            exit(1);
        }
    }
}

// Load packets into the window.
int load_packets_into_window(int done) {

    // Loop until we have have at most MAX_WINDOW_SIZE packets in the window for
    // this round.
    // 1 to 2001
    while (sequence - last_acked_packet < MAX_WINDOW_SIZE + 1) {

        // Load the packet into the window
        window[sequence % MAX_WINDOW_SIZE] = get_next_packet(sequence, &(packet_lengths[sequence % MAX_WINDOW_SIZE]));

        // The next packet doesn't contain anything. We have gotten all of
        // the packets from the file.
        if (window[sequence % MAX_WINDOW_SIZE] == NULL) {
            done = True;
            // mylog("[got all of the packets into window]\n");
            break;

        } else {

            // Increase the amount of packets we have added in total.
            sequence++;
        }
    }

    return done;
}

// Recieve the acknowledgements
void receive_acknowledgements(int sock, fd_set socks, struct timeval t, struct sockaddr_in in, socklen_t in_len) {

    // wait to receive, or for a timeout
    while (select(sock + 1, &socks, NULL, NULL, &t)) {

        unsigned char buf[10000];
        int buf_len = sizeof(buf);
        int received;
        if ((received = recvfrom(sock, &buf, buf_len, 0, (struct sockaddr *) &in, (socklen_t *) &in_len)) < 0) {
            perror("recvfrom");
            exit(1);
        }

        header *myheader = get_header(buf);

        // We got an ack and it's above our already noted last_acked_packet
        if ((myheader->magic == MAGIC) && (myheader->ack == True) && (myheader->sequence > last_acked_packet)) {
            mylog("[recv ack] %d\n", myheader->sequence);
            last_acked_packet = myheader->sequence;

        } else if (myheader -> sequence <= last_acked_packet) {
            // We got accounted for that ack already
            mylog("[recv ack below our noted last acked packet] %d\n", myheader->sequence);

        } else {
            // Otherwise it's just a corrupt packet with some other error
            mylog("[recv corrupted ack] %x %d\n", MAGIC, sequence);
        }

        FD_ZERO(&socks);
        FD_SET(sock, &socks);
    }
}

int main(int argc, char *argv[]) {
    /**
     * I've included some basic code for opening a UDP socket in C,
     * binding to a empheral port, printing out the port number.
     *
     * I've also included a very simple transport protocol that simply
     * acknowledges every received packet.  It has a header, but does
     * not do any error handling (i.e., it does not have sequence
     * numbers, timeouts, retries, a "window"). You will
     * need to fill in many of the details, but this should be enough to
     * get you started.
     */

    // extract the host IP and port
    if ((argc != 2) || (strstr(argv[1], ":") == NULL)) {
        usage();
    }

    char *tmp = (char *) malloc(strlen(argv[1])+1);
    strcpy(tmp, argv[1]);

    char *ip_s = strtok(tmp, ":");
    char *port_s = strtok(NULL, ":");

    // first, open a UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // next, construct the local port
    struct sockaddr_in out;
    out.sin_family = AF_INET;
    out.sin_port = htons(atoi(port_s));
    out.sin_addr.s_addr = inet_addr(ip_s);

    // socket for received packets
    struct sockaddr_in in;
    socklen_t in_len = sizeof(in);

    // construct the socket set
    fd_set socks;

    // construct the timeout
    struct timeval t;
    t.tv_sec = 30;
    t.tv_usec = 0;

    // Create the window. Allocate space for incoming TCP packets.
    for (unsigned int i = 0; i < MAX_WINDOW_SIZE; i++) {
        window[i] = malloc(TCP_PACKET_SIZE);
    }

    int done = False;

    // Loop until we have received the acks and there are no more packets to
    // send.
    while (True) {

        FD_ZERO(&socks);
        FD_SET(sock, &socks);
        t.tv_sec = 1;
        t.tv_usec = 0;

        // Load the packets into our window.
        done = load_packets_into_window(done);

        // Send them
        send_packets(sock, out);

        // Recieve acknowledgements
        receive_acknowledgements(sock, socks, t, in, in_len);

        // We have acknowleged all of our packets and there is nothing more
        // to read from the file.
        if (last_acked_packet == sequence && done) {
            mylog("[completed sending, except last packet]\n");
            break;
        }
    }

    send_final_packet(sock, out);
    mylog("[completed]\n");

    return 0;
}
