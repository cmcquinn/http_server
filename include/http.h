/**
 * @file http.h
 * @author Cameron McQuinn (cameron.mcquinn@gmail.com)
 * @brief http parsing module.
 * @version 0.1
 * @date 2020-01-28
 *
 * @copyright Copyright (c) 2020
 *
 */

#include <stdbool.h>

/**
 * @brief Check if a character buffer contains a valid http request.
 *
 * @param buf Pointer to a char buf.
 * @return true Buffer contains a valid http request.
 * @return false Buffer does not contain a valid http request.
 */
bool http_contains_valid_request(const char *buf);
