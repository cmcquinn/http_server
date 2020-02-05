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
#include <stddef.h>

#define HTTP_ERROR   -1 //!< Return code indicating error.
#define HTTP_SUCCESS 0  //!< Return code indicating success.

//! Enum representing the different possible HTTP methods.
typedef enum http_method {
    HTTP_GET,          //!< HTTP GET method
    HTTP_HEAD,         //!< HTTP HEAD method
    HTTP_POST,         //!< HTTP POST method
    HTTP_PUT,          //!< HTTP PUT method
    HTTP_DELETE,       //!< HTTP DELETE method
    HTTP_CONNECT,      //!< HTTP CONNECT method
    HTTP_OPTIONS,      //!< HTTP OPTIONS method
    HTTP_TRACE,        //!< HTTP TRACE method
    HTTP_PATCH,        //!< HTTP PATCH method
    HTTP_METHOD_COUNT, //!< Number of HTTP methods
    HTTP_METHOD_EMPTY, //!< No method found
} http_method_t;

//! Struct represending an HTTP message.
struct http_message {
    http_method_t method; /**< HTTP method specified in the message. */
    char *status;         //!< HTTP status field
    char *resource;       /**< Path to the requested resource. */
    char *header;         /**< HTTP header fields. */
    char *body;           /**< Message body */
    size_t body_len;      /**< Length of message body */
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
 * @param message Pointer to a struct http_message to be filled with the contents of the
 * message.
 * @return char* Pointer to the next character in \p buf after the HTTP message.
 */
char *http_extract_message(const char *buf, struct http_message *message);

/**
 * @brief Format a response to an HTTP message containing a request.
 *
 * @param message Pointer to an http_message struct containing a request.
 * @param response Pointer to an http_message struct which will be filled with the response to
 * the HTTP message represented by \p message.
 */
void http_prepare_response(struct http_message *message, struct http_message *response);

/**
 * @brief Format a struct http_message into a char buf.
 *
 * @param message Pointer to a struct http_message.
 * @return char* Pointer to a malloc'ed char buf containing a string representation of the
 * struct http_message.
 */
char *http_message_to_string(struct http_message *message);

/**
 * @brief Format the contents of a struct http_message into a char buf to allow them to be sent
 * over a connection.
 *
 * @param response Pointer to a struct http_message containing a response to an HTTP request.
 * @return char* Pointer to a char buffer containing a string representation of \p response.
 */
char *http_format_response(struct http_message *response);

/**
 * @brief Get the length in bytes of the response to a struct http_message.
 * 
 * @param response Pointer to a struct http_message containing a response to an HTTP request.
 * @return size_t Size of the formatted response.
 */
size_t http_get_response_len(struct http_message *response);

/**
 * @brief Initialize a struct http_message to default values;
 *
 * @param message Pointer to a struct http_message.
 */
void http_init_struct_message(struct http_message *message);

/**
 * @brief Free memory allocated for the fields of a struct http_message
 *
 * @param message Pointer to an http_message struct containing pointers to allocated memory.
 */
void http_free_struct_message(struct http_message *message);

#endif // HTTP_H