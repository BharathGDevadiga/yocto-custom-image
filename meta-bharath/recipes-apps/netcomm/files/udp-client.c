/*
 * udp-client.c
 *
 * A simple UDP client that sends datagrams to the UDP echo server
 * and prints the responses.
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
#include <sys/time.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 5001
#define BUFFER_SIZE  1024
#define RECV_TIMEOUT_SEC 3

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

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    /* Set receive timeout so we don't block forever */
    struct timeval tv;
    tv.tv_sec = RECV_TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);

    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", host);
        close(sockfd);
        return 1;
    }

    printf("[UDP-CLIENT] Sending to %s:%d\n", host, port);

    const char *messages[] = {
        "Hello from Yocto UDP client!\n",
        "Testing UDP datagram delivery\n",
        "meta-bharath netcomm recipe works!\n",
        NULL
    };

    char buffer[BUFFER_SIZE];

    for (int i = 0; messages[i] != NULL; i++) {
        printf("[UDP-CLIENT] Sending: %s", messages[i]);
        sendto(sockfd, messages[i], strlen(messages[i]), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr));

        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        ssize_t bytes = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                  (struct sockaddr *)&from_addr, &from_len);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("[UDP-CLIENT] Server replied: %s", buffer);
        } else {
            printf("[UDP-CLIENT] No response (timeout)\n");
        }
        sleep(1);
    }

    printf("[UDP-CLIENT] Done.\n");
    close(sockfd);
    return 0;
}
