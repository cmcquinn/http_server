#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

#define HTTP_ERROR -1
#define HTTP_SUCCESS 0

/**
 * @brief Initialize the http server.
 * 
 * @param port Port to listen for connections on.
 * @return int HTTP_ERROR on error, HTTP_SUCCESS otherwise.
 */
int http_server_init(const char* port);

/**
 * @brief Set length in bytes for recieving data.
 *
 * @param len Length in bytes.
 */
void http_set_recv_len(size_t len);

/**
 * @brief Get the recieve length in bytes.
 *
 */
size_t http_get_recv_len();

#endif // HTTP_H