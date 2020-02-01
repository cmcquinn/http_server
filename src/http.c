/**
 * @file http.c
 * @author Cameron McQuinn (cameron.mcquinn@gmail.com)
 * @brief HTTP parsing module
 * @version 0.1
 * @date 2020-01-28
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "http.h"
#include <string.h>

#define HTTP_GET_STR     "GET"     //!< HTTP GET method
#define HTTP_HEAD_STR    "HEAD"    //!< HTTP HEAD method
#define HTTP_POST_STR    "POST"    //!< HTTP POST method
#define HTTP_PUT_STR     "PUT"     //!< HTTP PUT method
#define HTTP_DELETE_STR  "DELETE " //!< HTTP DELETE method
#define HTTP_CONNECT_STR "CONNECT" //!< HTTP CONNECT method
#define HTTP_OPTIONS_STR "OPTIONS" //!< HTTP OPTIONS method
#define HTTP_TRACE_STR   "TRACE"   //!< HTTP TRACE method
#define HTTP_PATCH_STR   "PATCH"   //!< HTTP PATCH method

/**
 * @brief Check if a character buffer contains a valid HTTP request.
 *
 * @param buf Pointer to a char buf.
 * @return true Buffer contains a valid HTTP request.
 * @return false Buffer does not contain a valid HTTP request.
 */
bool http_contains_valid_message(const char *buf) {
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

/**
 * @brief Extract the HTTP method from a char buf containing an HTTP message and put it into a
 * struct http_message.
 *
 * @param buf Char buf containing a valid HTTP message.
 * @param message Pointer to a struct http_message to be filled with the contents of the message.
 * @return const char* Pointer to next character after the end of the HTTP method in \p buf if an
 * HTTP method is found, NULL otherwise.
 */
static const char *http_get_method(const char *buf, struct http_message *message) {
    const char *pch = NULL;
    if ((pch = strstr(buf, HTTP_GET_STR)) != NULL) {
        message->method = HTTP_GET;
    } else if ((pch = strstr(buf, HTTP_HEAD_STR)) != NULL) {
        message->method = HTTP_HEAD;
    } else if ((pch = strstr(buf, HTTP_POST_STR)) != NULL) {
        message->method = HTTP_POST;
    } else if ((pch = strstr(buf, HTTP_PUT_STR)) != NULL) {
        message->method = HTTP_PUT;
    } else if ((pch = strstr(buf, HTTP_DELETE_STR)) != NULL) {
        message->method = HTTP_DELETE;
    } else if ((pch = strstr(buf, HTTP_CONNECT_STR)) != NULL) {
        message->method = HTTP_CONNECT;
    } else if ((pch = strstr(buf, HTTP_OPTIONS_STR)) != NULL) {
        message->method = HTTP_OPTIONS;
    } else if ((pch = strstr(buf, HTTP_TRACE_STR)) != NULL) {
        message->method = HTTP_TRACE;
    } else if ((pch = strstr(buf, HTTP_PATCH_STR)) != NULL) {
        message->method = HTTP_PATCH;
    } else {
        message->method = HTTP_METHOD_NONE;
    }
    return strstr(pch, " "); // find space after method
}

/**
 * @brief Extract an HTTP message from a char buf and put it into a struct http_message.
 *
 * @param buf Char buf containing a valid HTTP message.
 * @param message Pointer to a struct http_message to be filled with the contents of the message.
 * @return int HTTP_SUCCESS if \p buf contains a valid HTTP message, HTTP_ERROR otherwise.
 */
int http_extract_message(const char *buf, struct http_message *message) {
    const char *pch;
    pch = http_get_method(buf, message);
}

/**
 * @brief Format a response to an HTTP message containing a request.
 *
 * @param message Pointer to an http_message struct containing a request.
 * @param response Pointer to an http_message struct which will be filled with the response to the
 * HTTP message represented by \p message.
 * @return int HTTP_SUCCESS if \p message contains a valid request, HTTP_ERROR otherwise.
 */
int http_prepare_response(struct http_message *message, struct http_message *response);