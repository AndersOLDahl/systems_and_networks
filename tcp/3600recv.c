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

// The first packet we will get will have a sequence number of 1 from send.
unsigned int sequence = 1;

int main();
void write_data(header * myheader, char * data, int buf_len);
void handle_received_packet(void * buf, int buf_len, int sock, struct sockaddr_in in);
void receive_packet(int sock, fd_set socks, struct timeval t, struct sockaddr_in in, socklen_t in_len, int buf_len, void * buf);

char all_data[MAX_WINDOW_SIZE * TCP_PACKET_SIZE];
unsigned int data_lengths[MAX_WINDOW_SIZE];

// Write out the data from the packet
void write_data(header * myheader, char * data, int buf_len) {

    // The sequence of the received packet is the same as the next one we are
    // looking for
    if (myheader->sequence == sequence) {

        // Write data out
        write(1, data, myheader->length);

        // Increase the sequence
        sequence++;

        unsigned int i = sequence % MAX_WINDOW_SIZE;

        mylog("[recv data] %d (%d) %s\n", myheader->sequence, myheader->length, "ACCEPTED (in-order)");

        // Loop through and write the data that is saved in our window up until the
        // next missing packet.
        while(data_lengths[i] > 0) {

            // We write the data
            write(1, &all_data[i * buf_len], data_lengths[i]);

            // Reset the size to zero so that the data can be overwritten later
            // on. We know we are done with it now.
            data_lengths[i] = 0;

            // Keep looping until the next missing packet is found.
            sequence++;
            i = sequence % MAX_WINDOW_SIZE;
        }
    }

    // If the incoming packet is greater than our sequence, then we add it
    // to the window and write it out later.
    else if (myheader->sequence >= sequence) {

        unsigned int i = myheader->sequence % MAX_WINDOW_SIZE;

        // We haven't added the data yet.
        if (data_lengths[i] == 0) {

            // Set the length and copy over the data.
            data_lengths[i] = myheader->length;
            memcpy(&all_data[i * buf_len], data, myheader->length);

            mylog("[recv data] %d (%d) %s\n", myheader->sequence, myheader->length, "ACCEPTED (out-of-order)");
        } else {
            // If we have already added it to the window then we ignore it.
            mylog("[recv data] %d (%d) %s\n", myheader->sequence, myheader->length, "IGNORED");
        }

        // The packet sequence is below our sequence. We have already
        // handled that packet.
    } else {
        mylog("[recv data] %d (%d) %s\n", myheader->sequence, myheader->length, "IGNORED");
    }
}

void handle_received_packet(void * buf, int buf_len, int sock, struct sockaddr_in in) {

    header *myheader = get_header(buf);
    char *data = get_data(buf);

    // dump_packet(buf, received);

    if (myheader->magic == MAGIC) {

        // We write the data in the packet if necessary, save it to the window, or
        // ignore it
        write_data(myheader, data, buf_len);

        // Send the acknowledgement. We have recieved up to that and including
        // that sequence number
        mylog("[send ack] %d\n", sequence);

        header *responseheader = make_header(sequence, 0, myheader -> eof, 1);
        if (sendto(sock, responseheader, sizeof(header), 0, (struct sockaddr *) &in, (socklen_t) sizeof(in)) < 0) {
            perror("sendto");
            exit(1);
        }

        // If we are at eof
        if (myheader->eof) {
            mylog("[completed]\n");
            exit(0);
        }

        // Otherwise something is wrong with the packet
    } else {
        mylog("[recv corrupt packet]\n");
    }
}

// Receive the packet and then acknowledge back that we got it.
void receive_packet(int sock, fd_set socks, struct timeval t, struct sockaddr_in in, socklen_t in_len, int buf_len, void * buf) {

    // Receive the packet
    if (select(sock + 1, &socks, NULL, NULL, &t)) {
        int received;
        if ((received = recvfrom(sock, buf, buf_len, 0, (struct sockaddr *) &in, (socklen_t *) &in_len)) < 0) {
            perror("recvfrom");
            exit(1);
        }

        // Send the ack and handle the packet
        handle_received_packet(buf, buf_len, sock, in);

        // Otherwise we timed out
    } else {
        mylog("[timeout]\n");
        exit(1);
    }
}

int main() {
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

    // first, open a UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // next, construct the local port
    struct sockaddr_in out;
    out.sin_family = AF_INET;
    out.sin_port = htons(0);
    out.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *) &out, sizeof(out))) {
        perror("bind");
        exit(1);
    }

    struct sockaddr_in tmp;
    int len = sizeof(tmp);
    if (getsockname(sock, (struct sockaddr *) &tmp, (socklen_t *) &len)) {
        perror("getsockname");
        exit(1);
    }

    mylog("[bound] %d\n", ntohs(tmp.sin_port));

    // wait for incoming packets
    struct sockaddr_in in;
    socklen_t in_len = sizeof(in);

    // construct the socket set
    fd_set socks;

    // construct the timeout
    struct timeval t;
    t.tv_sec = 30;
    t.tv_usec = 0;

    // our receive buffer
    int buf_len = TCP_PACKET_SIZE;
    void *buf = malloc(buf_len);

    while (True) {

        FD_ZERO(&socks);
        FD_SET(sock, &socks);

        // Receive the packets
        receive_packet(sock, socks, t, in, in_len, buf_len, buf);

    }

    return 0;
}
