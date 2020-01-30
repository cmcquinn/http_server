/**
 * @file server.c
 * @author Cameron McQuinn (cameron.mcquinn@gmail.com)
 * @brief Basic http server implemented in C
 * @version 0.1
 * @date 2020-01-24
 *
 * @copyright Copyright (c) 2020
 *
 */

#include <errno.h>
#include <malloc.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http.h"

#define DEFAULT_RECV_LEN  1024 //!< Default length for recieve operations
#define BACKLOG           5    //!< Number of connections allowed on the incoming queue
#define SLEEPTIME_SECONDS 5 //!< Time in seconds that each thread should sleep for before returning

#define handle_error_en(en, msg)                                                                   \
    do {                                                                                           \
        errno = en;                                                                                \
        perror(msg);                                                                               \
        exit(EXIT_FAILURE);                                                                        \
    } while (0)

static size_t recieve_len = DEFAULT_RECV_LEN;
static struct addrinfo *servinfo;
static int sock;

/**
 * @brief Initialize the http server.
 *
 * @param port Port to listen for connections on.
 */
void server_init(char *port) {
    int status;
    struct addrinfo hints;

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family   = AF_UNSPEC;   // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags    = AI_PASSIVE;  // fill in my IP for me

    if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // setup the socket
    if ((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        freeaddrinfo(servinfo);
        exit(1);
    }

    // lose the pesky "Address already in use" error message
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) == -1) {
        perror("setsockopt");
        exit(1);
    }

    // bind the socket to a port
    if (bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        fprintf(stderr, "bind error: %s\n", strerror(errno));
        exit(1);
    }
}

/**
 * @brief Routine for threads to handle connections.
 *
 * @param fd Socket file descriptor.
 */
void *connection_worker(void *fd) {
    char *buf = (char *)malloc(recieve_len * sizeof(char));
    int _fd   = (int)fd;
    recv(_fd, buf, recieve_len, 0);
    close(_fd);
    printf("Got message %s\n", buf);
    free(buf);

    // Sleep for a bit to simulate doing real work
    printf("Sleeping");
    for (int i = 0; i < SLEEPTIME_SECONDS; i++) {
        sleep(1);
        printf(".");
        fflush(stdout);
    }
    printf("\n");
    pthread_exit(NULL);
}

/**
 * @brief Busy loop that waits for events and creates threads to handle them.
 *
 */
void server_spin() {
    // listen for connections
    listen(sock, BACKLOG);
    pthread_t threadid;
    pthread_attr_t attr;

    // setup pthread attributes
    pthread_attr_init(&attr);
    int err;
    if ((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0)
        handle_error_en(err, "pthread_attr_setdetachstate");

    while (1) {
        // accept connections
        struct sockaddr_storage their_addr;
        socklen_t addr_size;
        int conn_fd;
        addr_size = sizeof(their_addr);

        if ((conn_fd = accept(sock, (struct sockaddr *)&their_addr, &addr_size)) == -1) {
            perror("accept");
            exit(1);
        }

        // dispatch a thread to handle the connection
        pthread_create(&threadid, &attr, connection_worker, (void *)conn_fd);
    }
}

/**
 * @brief Cleans up the server.
 *
 */
void server_exit() {
    freeaddrinfo(servinfo); // free the linked-list
}

/**
 * @brief Set length in bytes for recieving data.
 *
 * @param len Length in bytes.
 */
void server_set_recv_len(size_t len) {
    recieve_len = len;
}

/**
 * @brief Get the recieve length in bytes.
 *
 */
size_t server_get_recv_len() {
    return recieve_len;
}
