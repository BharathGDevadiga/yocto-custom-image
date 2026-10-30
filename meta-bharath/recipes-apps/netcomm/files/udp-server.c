/*
 * udp-server.c
 *
 * A simple UDP echo server.
 * Listens on a configurable port and echoes back datagrams
 * with a prefix.
 *
 * Author: Bharath
 * License: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEFAULT_PORT 5001
#define BUFFER_SIZE  1024

static volatile int running = 1;

static void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}

int main(int argc, char *argv[])
{
    int port = DEFAULT_PORT;
    if (argc > 1)
        port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    printf("[UDP-SERVER] Listening on port %d...\n", port);

    char buffer[BUFFER_SIZE];
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        ssize_t bytes = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                  (struct sockaddr *)&client_addr, &client_len);
        if (bytes < 0) {
            if (errno == EINTR) continue;
            perror("recvfrom");
            break;
        }
        buffer[bytes] = '\0';

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("[UDP-SERVER] From %s:%d => %s",
               client_ip, ntohs(client_addr.sin_port), buffer);

        /* Echo back with prefix */
        char reply[BUFFER_SIZE + 32];
        snprintf(reply, sizeof(reply), "[UDP-ECHO] %s", buffer);
        sendto(sockfd, reply, strlen(reply), 0,
               (struct sockaddr *)&client_addr, client_len);
    }

    printf("[UDP-SERVER] Shutting down.\n");
    close(sockfd);
    return 0;
}
