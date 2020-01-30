/**
 * @file main.c
 * @author Cameron McQuinn (cameron.mcquinn@gmail.com)
 * @brief
 * @version 0.1
 * @date 2020-01-16
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "http.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_PORT "1024"
#define UNDERLINE(x) "\033[4m" x "\033[m"

void usage(char *argv[]) {
    printf("Usage: %s [options] file\n", argv[0]);
    printf(" -p " UNDERLINE("PORT") "\tListen for connections on " UNDERLINE(
               "PORT") ". Default is port %s\n",
           DEFAULT_PORT);
    printf(" -s " UNDERLINE("SIZE") "\tSet recieve size in bytes\n");
}

int main(int argc, char *argv[]) {
    int opt;
    char *port = DEFAULT_PORT;
    unsigned int size;

    while ((opt = getopt(argc, argv, ":p:s:")) != -1) {
        switch (opt) {
            case 'p':
                port = optarg;
                break;

            case 's':
                size = (int)atoi(optarg);
                break;

            default:
                usage(argv);
                exit(EXIT_FAILURE);
                break;
        }
    }

    server_init(port);
    server_set_recv_len(size);
    server_spin();
    server_exit();
    return 0;
}