/*
 * tcp-server.c
 *
 * A simple TCP echo server.
 * Listens on a configurable port, accepts one client at a time,
 * echoes back whatever the client sends (with a prefix), and
 * logs all activity to stdout.
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

#define DEFAULT_PORT 5000
#define BUFFER_SIZE  1024
#define BACKLOG      5

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

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("[TCP-SERVER] Listening on port %d...\n", port);

    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("[TCP-SERVER] Client connected: %s:%d\n",
               client_ip, ntohs(client_addr.sin_port));

        char buffer[BUFFER_SIZE];
        ssize_t bytes;
        while ((bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytes] = '\0';
            printf("[TCP-SERVER] Received: %s", buffer);

            /* Echo back with a prefix */
            char reply[BUFFER_SIZE + 32];
            snprintf(reply, sizeof(reply), "[ECHO] %s", buffer);
            send(client_fd, reply, strlen(reply), 0);
        }

        printf("[TCP-SERVER] Client disconnected: %s\n", client_ip);
        close(client_fd);
    }

    printf("[TCP-SERVER] Shutting down.\n");
    close(server_fd);
    return 0;
}
