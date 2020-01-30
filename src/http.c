/**
 * @file http.c
 * @author Cameron McQuinn (cameron.mcquinn@gmail.com)
 * @brief http parsing module
 * @version 0.1
 * @date 2020-01-28
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "http.h"
#include <string.h>

/**
 * @brief Check if a character buffer contains a valid http request.
 *
 * @param buf Pointer to a char buf.
 * @return true Buffer contains a valid http request.
 * @return false Buffer does not contain a valid http request.
 */
bool http_contains_valid_request(const char *buf) {
    const char *ptr;

    // find GET
    if ((ptr = strstr(buf, "GET ")) == NULL)
        return false;

    // find HTTP/1.1
    if ((ptr = strstr(ptr, "HTTP/1.1\r\n")) == NULL)
        return false;

    // find Host field
    if ((ptr = strstr(ptr, "Host: ")) == NULL)
        return false;

    // find end of host field line
    if ((ptr = strstr(ptr, "\r\n")) == NULL)
        return false;

    // at the point we have a minimal HTTP/1.1 message
    return true;
}