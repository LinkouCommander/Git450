#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "serverM.h"

#define BUFFER_SIZE 1024

int set_udp_socket() {
    int sockfd;
    struct sockaddr_in udp_server_address;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&udp_server_address, 0, sizeof(udp_server_address));
    memset(&udp_client_address, 0, sizeof(udp_client_address));

    udp_server_address.sin_family = AF_INET; // IPv4
    udp_server_address.sin_addr.s_addr = INADDR_ANY;
    udp_server_address.sin_port = htons(serverM_UDP_PORT);

    if (bind(sockfd, (const struct sockaddr *)&udp_server_address, sizeof(udp_server_address)) < 0) {
        perror("UDP Bind failed");
        close(serverM_UDP_PORT);
        exit(EXIT_FAILURE);
    }

    udp_client_address.sin_family = AF_INET;
    udp_client_address.sin_port = htons(serverA_UDP_PORT);

    return sockfd;
}

int set_tcp_socket() {
    int sockfd;
    struct sockaddr_in address;
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("TCP Socket creation failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(serverM_TCP_PORT);

    if (bind(sockfd, (const struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("TCP Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if(listen(sockfd, 3) < 0) {
        perror("Listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int set_tcp_client_socket(int tcp_server_socket) {
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    int sockfd;
    if ((sockfd = accept(tcp_server_socket, (struct sockaddr*)&address, (socklen_t*)&addr_len)) < 0) {
        perror("TCP Accept failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int main() {
    int udp_socket = set_udp_socket();

    // printf("Message sent to serverA\n");

    int tcp_server_socket = set_tcp_socket();
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    printf("Server M is up and running using UDP on port %d.\n", serverM_UDP_PORT);

    while(1) {
        char buffer[BUFFER_SIZE] = {0};
        char username[100], password[100];
        memset(buffer, 0, BUFFER_SIZE);

        int tcp_client_socket = set_tcp_client_socket(tcp_server_socket);
        // printf("TCP client connected.\n");

        if(recv(tcp_client_socket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("TCP Receive failed");
            exit(EXIT_FAILURE);
        }
        strcpy(username, buffer);
        // printf("client username: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);

        if(recv(tcp_client_socket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("TCP Receive failed");
            exit(EXIT_FAILURE);
        }
        strcpy(password, buffer);
        // printf("client password: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);

        printf("Server M has received username %s and password ****.\n", username);

        int authenticationCode;

        if(strcmp(username, "guest") == 0 && strcmp(password, "guest") == 0) {
            authenticationCode = 1;
        }
        else {
            memset(buffer, 0, BUFFER_SIZE);

            sendto(udp_socket, username, strlen(username), 0, (struct sockaddr *)&udp_client_address, udp_client_len);
            sendto(udp_socket, password, strlen(password), 0, (struct sockaddr *)&udp_client_address, udp_client_len);
            printf("Server M has sent authentication request to Server A\n");

            recvfrom(udp_socket, &authenticationCode, sizeof(authenticationCode), 0, (struct sockaddr *)&udp_client_address, (socklen_t*)&udp_client_len);
            printf("The main server has received the response from server A using UDP over %d.\n", serverA_UDP_PORT);
        }
        send(tcp_client_socket, &authenticationCode, sizeof(authenticationCode), 0);
        printf("The main server has sent the response from server A to client using TCP over port %d.\n", serverM_TCP_PORT);

        close(tcp_client_socket);
    }
    close(udp_socket);
    close(tcp_server_socket);
}