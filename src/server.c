/**
 * @file server.c
 * @author Cameron McQuinn (cameron.mcquinn@gmail.com)
 * @brief Basic HTTP server implemented in C
 * @version 0.1
 * @date 2020-01-24
 *
 * @copyright Copyright (c) 2020
 *
 */

#include <arpa/inet.h>
#include <errno.h>
#include <malloc.h>
#include <netdb.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "http.h"

#define DEFAULT_RECV_LEN  1024 //!< Default length for recieve operations
#define BACKLOG           5    //!< Number of connections allowed on the incoming queue
#define SLEEPTIME_SECONDS 5 //!< Time in seconds that each thread should sleep for before returning
#define NULL_TERM_LEN     1 //!< Length of null terminator

#define thread_debug(__format, ...)                                                                \
    if (verbose) {                                                                                 \
        printf("%ld: " __format, syscall(__NR_gettid), ##__VA_ARGS__);                             \
    }
#define debug_print(__format, ...)                                                                 \
    if (verbose) {                                                                                 \
        printf(__format, ##__VA_ARGS__);                                                           \
    }

static size_t recieve_len = DEFAULT_RECV_LEN;
static struct addrinfo *servinfo;
static int sock;
static bool verbose = false;

// get sockaddr, IPv4 or IPv6:
static void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

/**
 * @brief Send data to a socket.
 *
 * @param s Socket to send data to.
 * @param buf Buffer containing data to send.
 * @param len Length of data to be sent
 * @return int -1 on failure, 0 on success
 */
static int sendall(int s, char *buf, size_t *len) {
    size_t total     = 0;    // how many bytes we've sent
    size_t bytesleft = *len; // how many we have left to send
    ssize_t n;

    while (total < *len) {
        n = send(s, buf + total, bytesleft, 0);
        if (n == -1) {
            break;
        }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

/**
 * @brief Initialize the HTTP server.
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
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    // setup the socket
    if ((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
        perror("socket");
        freeaddrinfo(servinfo);
        exit(EXIT_FAILURE);
    }

    // lose the pesky "Address already in use" error message
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // bind the socket to a port
    if (bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Routine for threads to handle connections.
 *
 * @param fd Socket file descriptor.
 */
void *connection_worker(void *fd) {
    char *buf                  = NULL; // buffer to recieve into
    char *head                 = NULL; // pointer to end of processed part of buffer
    char *slot                 = NULL; // pointer to the next location to recieve into
    unsigned int recieve_count = 0;
    int _fd                    = *(int *)fd;
    bool connection_closed     = false;

    buf  = (char *)malloc(recieve_len + NULL_TERM_LEN); // initial memory allocation
    head = buf;

    do {
        do {
            size_t len         = (recieve_count + 1) * recieve_len + NULL_TERM_LEN;
            size_t head_offset = head - buf; // save offset of head ptr from start of buffer
            if ((buf = realloc(buf, 2 * len)) == NULL) // allocate memory for another iteration
                perror("realloc");

            head = buf + head_offset;
            slot = head +
                   recieve_count * recieve_len; // get pointer to start of newly allocated memory
            memset(slot, '\0', recieve_len + NULL_TERM_LEN);

            size_t bytes = 0;
            if ((bytes = recv(_fd, slot, recieve_len, 0)) <= 0) {
                if (bytes == 0) {
                    connection_closed = true;
                } else {
                    perror("recv");
                }
                break;
            }
            debug_print("%s", slot);
            recieve_count++;
        } while (!http_contains_valid_message(head));

        if (!connection_closed) {
            struct http_message msg;
            http_init_struct_message(&msg);
            head = http_extract_message(head, &msg);

            char *msg_str = http_message_to_string(&msg);
            thread_debug("Got message %s after %u transactions\n", msg_str, recieve_count);
            free(msg_str);

            // create response
            struct http_message rsp;
            http_init_struct_message(&rsp);
            http_prepare_response(&msg, &rsp);
            char *rsp_msg       = http_format_response(&rsp);
            size_t response_len = strlen(rsp_msg);
            thread_debug("Sending response: %s\n", rsp_msg);

            // send response
            sendall(_fd, rsp_msg, &response_len);
            sendall(_fd, rsp.body, &rsp.body_len);

            free(rsp_msg);
            http_free_struct_message(&msg);
            http_free_struct_message(&rsp);
        }

    } while (!connection_closed);

    free(buf);
    close(_fd);

    // Sleep for a bit to simulate doing real work
    thread_debug("Sleeping\n");
    sleep(SLEEPTIME_SECONDS);
    return NULL;
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
    if ((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) {
        perror("pthread_attr_setdetachstate");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // accept connections
        struct sockaddr_storage their_addr;
        socklen_t addr_size;
        int conn_fd;
        addr_size = sizeof(their_addr);

        if ((conn_fd = accept(sock, (struct sockaddr *)&their_addr, &addr_size)) == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // report connection
        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), ipstr,
                  sizeof ipstr);
        debug_print("server: got connection from %s\n", ipstr);

        // dispatch a thread to handle the connection
        pthread_create(&threadid, &attr, connection_worker, (void *)&conn_fd);
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

/**
 * @brief Put the server into verbose mode, which prints out more information useful for debugging.
 *
 */
void server_set_verbose_mode() {
    verbose = true;
}