#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define UDP_PORT 8080
#define BUFFER_SIZE 1024

int set_udp_socket() {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int main() {
    int udp_socket = set_udp_socket();
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(UDP_PORT);

    const char *message = "Hello, server!";
    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *)&server_addr, server_len);
    printf("Initial message sent to server.\n");

    char buffer[BUFFER_SIZE];
    while (1) {
        recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
        buffer[strlen(buffer)] = '\0';
        printf("Received from server: %s\n", buffer);
    }

    close(udp_socket);
    return 0;
}
