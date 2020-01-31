/**
 * @file server.h
 * @author Cameron McQuinn (cameron.mcquinn@gmail.com)
 * @brief Function prototypes for a basic HTTP server implemented in C.
 * @version 0.1
 * @date 2020-01-24
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>

#define SERVER_ERROR   -1 //!< Return code indicating error.
#define SERVER_SUCCESS 0  //!< Return code indicating success.

/**
 * @brief Initialize the HTTP server.
 *
 * @param port Port to listen for connections on.
 */
void server_init(char *port);

/**
 * @brief Busy loop that waits for events and creates threads to handle them.
 *
 */
void server_spin();

/**
 * @brief Cleans up the server and exits.
 *
 */
void server_exit();

/**
 * @brief Set length in bytes for recieving data.
 *
 * @param len Length in bytes.
 */
void server_set_recv_len(size_t len);

/**
 * @brief Get the recieve length in bytes.
 *
 */
size_t server_get_recv_len();

#endif // SERVER_H