/**
 * @f http.c
 * @author Cameron McQuinn (cameron.mcquinn@gmail.com)
 * @brief HTTP parsing module
 * @version 0.1
 * @date 2020-01-28
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "http.h"
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define HTTP_HOST_FIELD   "Host: "                      //!< HTTP Host field name
#define HTTP_LINE_END     "\r\n"                        //!< End of line in HTTP message
#define HTTP_STATUS_OK    "HTTP/1.1 200 OK"             //!< HTTP OK status code
#define HTTP_STATUS_ERROR "HTTP/1.1 404 File Not Found" //!< HTTP error status code

//! Array with the string contents of the HTTP methods in order of their appearance in enum
//! http_method.
static const char *const http_methods[] = {
    "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH",
};

// function prototypes
static const char *http_get_method(const char *buf, struct http_message *message);
static const char *http_get_resource(const char *buf, struct http_message *message);
static const char *http_get_host(const char *buf, struct http_message *message);

/**
 * @brief Check if a character buffer contains a valid HTTP request.
 *
 * @param buf Pointer to a char buf.
 * @return true Buffer contains a valid HTTP request.
 * @return false Buffer does not contain a valid HTTP request.
 */
bool http_contains_valid_message(const char *buf) {
    const char *ptr;

    // find HTTP method
    struct http_message msg;
    http_init_struct_message(&msg);
    if ((ptr = http_get_method(buf, &msg)) == NULL)
        return false;

    // find HTTP/1.1
    if ((ptr = strstr(ptr, "HTTP/1.1" HTTP_LINE_END)) == NULL)
        return false;

    // find Host field
    if ((ptr = strstr(ptr, "Host: ")) == NULL)
        return false;

    // find end of host field line
    if ((ptr = strstr(ptr, HTTP_LINE_END)) == NULL)
        return false;

    // at the point we have a minimal HTTP/1.1 message
    return true;
}

/**
 * @brief Extract the HTTP method from a char buf containing an HTTP message and put it into a
 * struct http_message.
 *
 * @param buf Char buf containing a valid HTTP message.
 * @param message Pointer to a struct http_message. The \p method field of \p message will be set.
 * @return const char* Pointer to next character after the end of the HTTP method in \p buf if an
 * HTTP method is found, NULL otherwise.
 */
static const char *http_get_method(const char *buf, struct http_message *message) {
    const char *pch = NULL;
    message->method = HTTP_METHOD_EMPTY;

    for (int i = 0; i < HTTP_METHOD_COUNT; i++) {
        if ((pch = strstr(buf, http_methods[i])) != NULL) {
            message->method = i;
            break;
        }
    }

    return message->method != HTTP_METHOD_EMPTY ? strstr(pch, " ")
                                                : NULL; // find space after method
}

/**
 * @brief Extract the resource requested from a char buf containing an HTTP message and put it into
 * a struct http_message.
 *
 * @param buf Char buf containing a valid HTTP message.
 * @param message Pointer to a struct http_message. The \p resource field of \p message will be set.
 * @return const char* Pointer to next character after the resource path in \p buf .
 */
static const char *http_get_resource(const char *buf, struct http_message *message) {
    const char *start = strchr(buf, '/'); // find '/' at beginning of path

    // find end of request path
    const char *end = start;
    while (*++end != ' ')
        ;

    // set resource field of struct http_message
    size_t len        = end - start;
    message->resource = (char *)malloc(len + 1); // len + 1 bytes to fit null terminator
    memset(message->resource, '\0', len + 1);    // initialize allocated memory
    strncat(message->resource, start, len);      // copy resource field and append null terminator

    return end;
}

/**
 * @brief Extract the hostname from a char buf containing an HTTP message and put it into a struct
 * http_message.
 *
 * @param buf Char buf containing a valid HTTP message.
 * @param message Pointer to a struct http_message. The \p headers field of \p message will be
 * modified.
 * @return const char* Pointer to next character after the hostname in \p buf .
 */
static const char *http_get_host(const char *buf, struct http_message *message) {
    const char *start = strstr(buf, HTTP_HOST_FIELD); // find start of Host field
    start             = strchr(start, ' ');           // find space before hostname
    start++;                                          // get pointer to first char of hostname
    const char *end = strstr(start, HTTP_LINE_END);   // find end of line

    // set header field of struct http_message
    size_t len      = end - start;
    message->header = (char *)malloc(len + 1); // len + 1 bytes to fit null terminator
    memset(message->header, '\0', len + 1);    // initialize allocated memory
    strncat(message->header, start, len);      // copy host field and append null terminator

    // return pointer to next char after end of Host field
    return end + strlen(HTTP_LINE_END);
}

/**
 * @brief Extract an HTTP message from a char buf and put it into a struct http_message.
 *
 * @param buf Char buf containing a valid HTTP message.
 * @param message Pointer to a struct http_message to be filled with the contents of the message.
 * @return const char* Pointer to the next character in \p buf after the HTTP message.
 */
const char *http_extract_message(const char *buf, struct http_message *message) {
    const char *pch;
    pch = http_get_method(buf, message);
    pch = http_get_resource(pch, message);
    pch = http_get_host(pch, message);
    return pch;
}

/**
 * @brief Format a response to an HTTP message containing a request.
 *
 * @param message Pointer to an http_message struct containing a request.
 * @param response Pointer to an http_message struct which will be filled with the response to the
 * HTTP message represented by \p message.
 */
void http_prepare_response(struct http_message *message, struct http_message *response) {
    FILE *f;
    if ((f = fopen(message->resource, O_RDONLY)) == NULL) {
        perror("fopen");
        fprintf(stderr, "%s", strerror(errno));
        size_t status_size = strlen(HTTP_STATUS_ERROR) + 1;
        response->status   = (char *)malloc(status_size);
        memset(response->status, '\0', status_size); // initialize memory
        snprintf(response->status, status_size, "%s", HTTP_STATUS_ERROR);
    } else {
        size_t status_size = strlen(HTTP_STATUS_OK) + 1;
        response->status   = (char *)malloc(status_size);
        memset(response->status, '\0', status_size);
        snprintf(response->status, status_size, "%s", HTTP_STATUS_OK);
        fseek(f, 0L, SEEK_END);                     // seek end of f
        size_t body_size = ftell(f);                // get position in bytes
        rewind(f);                                  // rewind f to beginning
        response->body = (char *)malloc(body_size); // allocate memory for body
        memset(response->body, '\0', body_size);    // initialize memory
        fread(message->body, body_size, 1, f);
    }
}

void http_init_struct_message(struct http_message *message) {
    message->method   = HTTP_METHOD_EMPTY;
    message->status   = NULL;
    message->resource = NULL;
    message->header   = NULL;
    message->body     = NULL;
}

/**
 * @brief Free memory allocated for the fields of a struct http_message
 *
 * @param message Pointer to an http_message struct containing pointers to allocated memory.
 */
void http_free_struct_message(struct http_message *message) {
    if (message->status != NULL)
        free(message->status);
    
    if (message->resource != NULL)
        free(message->resource);

    if (message->header != NULL)
        free(message->header);

    if (message->body != NULL)
        free(message->body);
}