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
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define HTTP_HOST_FIELD       "Host: "                      //!< HTTP Host field name
#define HTTP_LINE_END         "\r\n"                        //!< End of line in HTTP message
#define HTTP_STATUS_OK        "HTTP/1.1 200 OK"             //!< HTTP OK status code
#define HTTP_STATUS_ERROR     "HTTP/1.1 404 File Not Found" //!< HTTP error status code
#define HTTP_CONTENT_LENGTH   "Content-Length: "            //!< HTTP content length header field
#define CLOSE_CONNECTION      "Connection: close" //!< Header to tell the server to close the connection
#define NULL_TERM_LEN         1                   //!< Length of null terminator
#define HTTP_STRUCT_MEM_COUNT 5                   //!< Number of members in struct http_message
#define IF_NOT_NULL(x)        (x != NULL ? x : "") //!< Pass pointer to sprintf only if not null

//! Array with the string contents of the HTTP methods in order of their appearance in enum
//! http_method.
static const char *const http_methods[] = {
    "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH",
};

// function prototypes
static char *http_get_method(const char *buf, struct http_message *message);
static char *http_get_resource(const char *buf, struct http_message *message);
static char *http_get_host(const char *buf, struct http_message *message);

static inline size_t get_method_len(http_method_t method) {
    return method != HTTP_METHOD_EMPTY ? strlen(http_methods[method]) : 0;
}

static inline const char *method_to_string(http_method_t method) {
    return method != HTTP_METHOD_EMPTY ? http_methods[method] : "";
}

static inline size_t count_digits(int i) {
    return (size_t)floor(log10(i) + 1);
}

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
 * @return char* Pointer to next character after the end of the HTTP method in \p buf if an
 * HTTP method is found, NULL otherwise.
 */
static char *http_get_method(const char *buf, struct http_message *message) {
    const char *pch = NULL;

    if (message->method == HTTP_METHOD_EMPTY) {
        for (int i = 0; i < HTTP_METHOD_COUNT; i++) {
            if ((pch = strstr(buf, http_methods[i])) != NULL) {
                message->method = i;
                break;
            }
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
 * @return char* Pointer to next character after the resource path in \p buf .
 */
static char *http_get_resource(const char *buf, struct http_message *message) {
    char *start = strchr(buf, '/'); // find '/' at beginning of path

    // find end of request path
    char *end = start;
    while (*++end != ' ')
        ;

    // set resource field of struct http_message
    size_t len        = end - start;
    message->resource = (char *)malloc(len + NULL_TERM_LEN);
    memset(message->resource, '\0', len + NULL_TERM_LEN); // initialize allocated memory
    strncat(message->resource, start, len); // copy resource field and append null terminator

    return end;
}

/**
 * @brief Extract the hostname from a char buf containing an HTTP message and put it into a struct
 * http_message.
 *
 * @param buf Char buf containing a valid HTTP message.
 * @param message Pointer to a struct http_message. The \p headers field of \p message will be
 * modified.
 * @return char* Pointer to next character after the hostname in \p buf .
 */
static char *http_get_host(const char *buf, struct http_message *message) {
    char *start = strstr(buf, HTTP_HOST_FIELD); // find start of Host field
    start       = strchr(start, ' ');           // find space before hostname
    start++;                                    // get pointer to first char of hostname
    char *end = strstr(start, HTTP_LINE_END);   // find end of line

    // set header field of struct http_message
    size_t len      = end - start;
    message->header = (char *)malloc(len + NULL_TERM_LEN);
    memset(message->header, '\0', len + NULL_TERM_LEN); // initialize allocated memory
    strncat(message->header, start, len); // copy host field and append null terminator

    // return pointer to next char after end of Host field
    return end + strlen(HTTP_LINE_END);
}

/**
 * @brief Extract an HTTP message from a char buf and put it into a struct http_message.
 *
 * @param buf Char buf containing a valid HTTP message.
 * @param message Pointer to a struct http_message to be filled with the contents of the message.
 * @return char* Pointer to the next character in \p buf after the HTTP message.
 */
char *http_extract_message(const char *buf, struct http_message *message) {
    char *pch;
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
    int f;
    char *dir  = get_current_dir_name();
    size_t len = strlen(dir) + strlen(message->resource) + NULL_TERM_LEN;
    char path[len];
    sprintf(path, "%s%s", dir, message->resource);
    if ((f = open(path, O_RDONLY)) < 0) {
        perror("open");

        // insert response code
        size_t status_size = strlen(HTTP_STATUS_ERROR) + NULL_TERM_LEN;
        response->status   = (char *)malloc(status_size);
        memset(response->status, '\0', status_size); // initialize memory
        snprintf(response->status, status_size, "%s", HTTP_STATUS_ERROR);

        // add header
        size_t header_size = strlen(CLOSE_CONNECTION) + strlen(HTTP_LINE_END) + NULL_TERM_LEN;
        response->header   = (char *)malloc(header_size);
        snprintf(response->header, header_size, "%s" HTTP_LINE_END, CLOSE_CONNECTION);
    } else {
        // insert response code
        size_t status_size = strlen(HTTP_STATUS_OK) + NULL_TERM_LEN;
        response->status   = (char *)malloc(status_size);
        memset(response->status, '\0', status_size);
        snprintf(response->status, status_size, "%s", HTTP_STATUS_OK);

        // copy file to body of http message
        size_t body_size   = lseek(f, 0L, SEEK_END); // seek end of file to get size in bytes
        response->body_len = body_size;
        lseek(f, 0L, SEEK_SET);                                     // seek beginning of file
        response->body = (char *)malloc(body_size + NULL_TERM_LEN); // allocate memory for body
        memset(response->body, '\0', body_size + NULL_TERM_LEN);    // initialize memory
        ssize_t bytes = 0;
        if ((bytes = read(f, response->body, body_size)) < 0) // copy data
            perror("read");

        // insert Content-Length header field
        size_t header_size = strlen(HTTP_CONTENT_LENGTH) + count_digits(body_size) +
                             strlen(HTTP_LINE_END) + NULL_TERM_LEN;
        response->header = (char *)malloc(header_size);
        snprintf(response->header, header_size, "%s%lu%s", HTTP_CONTENT_LENGTH, body_size,
                 HTTP_LINE_END);
    }
}

/**
 * @brief Format a struct http_message into a char buf.
 *
 * @param message Pointer to a struct http_message.
 * @return char* Pointer to a malloc'ed char buf containing a string representation of the
 * struct http_message.
 */
char *http_message_to_string(struct http_message *message) {
    size_t message_len = get_method_len(message->method);

    // space for fields
    if (message->header)
        message_len += strlen(message->header);
    if (message->resource)
        message_len += strlen(message->resource);
    if (message->status)
        message_len += strlen(message->status);
    if (message->body)
        message_len += strlen(message->body);

    message_len += HTTP_STRUCT_MEM_COUNT * strlen("\n"); // space for newlines after
                                                         // each field and null terminator at end

    char *message_str = (char *)malloc(message_len);
    snprintf(message_str, message_len, "%s %s %s %s %s", method_to_string(message->method),
             IF_NOT_NULL(message->status), IF_NOT_NULL(message->resource),
             IF_NOT_NULL(message->header), IF_NOT_NULL(message->body));
    return message_str;
}

/**
 * @brief Format the contents of a struct http_message into a char buf to allow them to be sent over
 * a connection.
 *
 * @param response Pointer to a struct http_message containing a response to an HTTP request.
 * @return char* Pointer to a char buffer containing a string representation of \p response.
 */
char *http_format_response(struct http_message *response) {
    // handle 404 error case
    if (strncmp(response->status, HTTP_STATUS_ERROR, strlen(HTTP_STATUS_ERROR)) == 0) {
        size_t message_size = strlen(response->status) + strlen(HTTP_LINE_END) +
                              strlen(response->header) + strlen(HTTP_LINE_END) + NULL_TERM_LEN;
        char *raw_message = (char *)malloc(message_size);
        snprintf(raw_message, message_size, "%s" HTTP_LINE_END "%s" HTTP_LINE_END, response->status,
                 response->header); // format message into char buf
        return raw_message;
    } else { // handle regular response
        size_t message_size = strlen(response->status) + strlen(HTTP_LINE_END) +
                              strlen(response->header) + strlen(HTTP_LINE_END) +
                              strlen(response->body) + strlen(HTTP_LINE_END) + NULL_TERM_LEN;
        char *raw_message = (char *)malloc(message_size);
        snprintf(raw_message, message_size,
                 "%s" HTTP_LINE_END "%s" HTTP_LINE_END "%s" HTTP_LINE_END, response->status,
                 response->header,
                 response->body); // format message into char buf
        return raw_message;
    }
}

/**
 * @brief Get the length in bytes of the response to a struct http_message.
 *
 * @param response Pointer to a struct http_message containing a response to an HTTP request.
 * @return size_t Size of the formatted response.
 */
size_t http_get_response_len(struct http_message *response) {
    size_t message_size = 0;
    // handle 404 error case
    if (strncmp(response->status, HTTP_STATUS_ERROR, strlen(HTTP_STATUS_ERROR)) == 0) {
        message_size = strlen(response->status) + strlen(HTTP_LINE_END) + strlen(response->header) +
                       strlen(HTTP_LINE_END) + NULL_TERM_LEN;

    } else { // handle regular response
        message_size = strlen(response->status) + strlen(HTTP_LINE_END) + strlen(response->header) +
                       strlen(HTTP_LINE_END) + strlen(response->body) + strlen(HTTP_LINE_END) +
                       NULL_TERM_LEN;
    }
    return message_size;
}

/**
 * @brief Initialize a struct http_message to default values;
 *
 * @param message Pointer to a struct http_message.
 */
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
