/**
 * @file http.h
 * @author Cameron McQuinn (cameron.mcquinn@gmail.com)
 * @brief HTTP parsing module.
 * @version 0.1
 * @date 2020-01-28
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef HTTP_H
#define HTTP_H

#include <stdbool.h>

#define HTTP_ERROR   -1 //!< Return code indicating error.
#define HTTP_SUCCESS 0  //!< Return code indicating success.

//! Enum representing the different possible HTTP methods.
typedef enum http_method {
    HTTP_GET,         //!< HTTP GET method
    HTTP_HEAD,        //!< HTTP HEAD method
    HTTP_POST,        //!< HTTP POST method
    HTTP_PUT,         //!< HTTP PUT method
    HTTP_DELETE,      //!< HTTP DELETE method
    HTTP_CONNECT,     //!< HTTP CONNECT method
    HTTP_OPTIONS,     //!< HTTP OPTIONS method
    HTTP_TRACE,       //!< HTTP TRACE method
    HTTP_PATCH,       //!< HTTP PATCH method
    HTTP_METHOD_NONE, //!< No HTTP method found
} http_method_t;

//! Struct represending an HTTP message.
struct http_message {
    http_method_t method; /**< HTTP method specified in the message. */
    const char *resource; /**< Path to the requested resource. */
    const char *header;   /**< HTTP header fields. */
    const char *body;     /**< Message body */
};

/**
 * @brief Check if a character buffer contains a valid HTTP request.
 *
 * @param buf Pointer to a char buf.
 * @return true Buffer contains a valid HTTP request.
 * @return false Buffer does not contain a valid HTTP request.
 */
bool http_contains_valid_message(const char *buf);

/**
 * @brief Extract an HTTP message from a char buf and put it into a struct http_message.
 *
 * @param buf Char buf containing a valid HTTP message.
 * @param message Pointer to a struct http_message to be filled with the contents of the message.
 * @return int HTTP_SUCCESS if \p buf contains a valid HTTP message, HTTP_ERROR otherwise.
 */
int http_extract_message(const char *buf, struct http_message *message);

/**
 * @brief Format a response to an HTTP message containing a request.
 *
 * @param message Pointer to an http_message struct containing a request.
 * @param response Pointer to an http_message struct which will be filled with the response to the
 * HTTP message represented by \p message.
 * @return int HTTP_SUCCESS if \p message contains a valid request, HTTP_ERROR otherwise.
 */
int http_prepare_response(struct http_message *message, struct http_message *response);

#endif // HTTP_H