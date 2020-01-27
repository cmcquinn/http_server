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

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "http.h"

#define DEFAULT_RECV_LEN 4

static size_t recvlen = DEFAULT_RECV_LEN;
static struct addrinfo *servinfo;

/**
 * @brief Initialize the http server.
 *
 * @param port Port to listen for connections on.
 * @return int HTTP_ERROR on error, HTTP_SUCCESS otherwise.
 */
int http_server_init(const char* port) {
    int status;
    struct addrinfo hints;

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family   = AF_UNSPEC;   // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags    = AI_PASSIVE;  // fill in my IP for me

    if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // servinfo now points to a linked list of 1 or more struct addrinfos

    // ... do everything until you don't need servinfo anymore ....

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
