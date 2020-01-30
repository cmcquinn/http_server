#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>

#define SERVER_ERROR   -1
#define SERVER_SUCCESS 0

/**
 * @brief Initialize the http server.
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