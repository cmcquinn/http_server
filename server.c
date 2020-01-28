/**
 * @file http.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http.h"

#define DEFAULT_RECV_LEN 4 //!< Default length for recieve operations
#define BACKLOG          5 //!< Number of connections allowed on the incoming queue

static size_t recvlen = DEFAULT_RECV_LEN;
static struct addrinfo *servinfo;
static int sock;

/**
 * @brief Initialize the http server.
 *
 * @param port Port to listen for connections on.
 */
void http_server_init(char *port) {
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

    // listen for connections
    listen(sock, BACKLOG);

    // accept connections
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    int conn_fd;
    addr_size = sizeof(their_addr);

    if ((conn_fd = accept(sock, (struct sockaddr *)&their_addr, &addr_size)) == -1) {
        perror("accept");
        exit(1);
    }

    char *buf = (char *)malloc(recvlen * sizeof(char));
    recv(conn_fd, buf, recvlen, 0);

    close(conn_fd);

    printf("Got message %s\n", buf);

    free(buf);

    // ... do everything until you don't need servinfo anymore ....
}

/**
 * @brief Cleans up the server.
 *
 */
void http_server_exit() {
    freeaddrinfo(servinfo); // free the linked-list
}

/**
 * @brief Set length in bytes for recieving data.
 *
 * @param len Length in bytes.
 */
void http_set_recv_len(size_t len) {
    recvlen = len;
}

/**
 * @brief Get the recieve length in bytes.
 *
 */
size_t http_get_recv_len() {
    return recvlen;
}
