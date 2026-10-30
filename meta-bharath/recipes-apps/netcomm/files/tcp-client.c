/*
 * tcp-client.c
 *
 * A simple TCP client that connects to the TCP echo server,
 * sends a message, and prints the server's response.
 *
 * Author: Bharath
 * License: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 5000
#define BUFFER_SIZE  1024

int main(int argc, char *argv[])
{
    const char *host = DEFAULT_HOST;
    int port = DEFAULT_PORT;

    if (argc > 1) host = argv[1];
    if (argc > 2) port = atoi(argv[2]);

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Usage: %s [host] [port]\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);

    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", host);
        close(sock);
        return 1;
    }

    printf("[TCP-CLIENT] Connecting to %s:%d...\n", host, port);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }
    printf("[TCP-CLIENT] Connected!\n");

    char buffer[BUFFER_SIZE];

    /* Send a few test messages */
    const char *messages[] = {
        "Hello from Yocto TCP client!\n",
        "Embedded Linux networking test\n",
        "Custom layer meta-bharath works!\n",
        NULL
    };

    for (int i = 0; messages[i] != NULL; i++) {
        printf("[TCP-CLIENT] Sending: %s", messages[i]);
        send(sock, messages[i], strlen(messages[i]), 0);

        ssize_t bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("[TCP-CLIENT] Server replied: %s", buffer);
        }
        sleep(1);
    }

    printf("[TCP-CLIENT] Done. Closing connection.\n");
    close(sock);
    return 0;
}
