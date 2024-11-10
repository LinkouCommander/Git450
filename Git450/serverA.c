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
    socklen_t addr_len = sizeof(server_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(UDP_PORT);

    char buffer[BUFFER_SIZE] = {0};

    while (1) {
        // 接收來自 UDP Server 的訊息
        if (recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len) < 0) {
            perror("UDP Receive failed");
            continue;
        }
        printf("Received message from UDP server: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);
    }

    close(udp_socket);
    return 0;
}
