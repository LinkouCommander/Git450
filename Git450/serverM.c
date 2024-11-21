#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "serverM.h"

int set_udp_socket() {
    int sockfd;
    struct sockaddr_in udp_server_address;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&udp_server_address, 0, sizeof(udp_server_address));

    udp_server_address.sin_family = AF_INET; // IPv4
    udp_server_address.sin_addr.s_addr = INADDR_ANY;
    udp_server_address.sin_port = htons(serverM_UDP_PORT);

    if (bind(sockfd, (const struct sockaddr *)&udp_server_address, sizeof(udp_server_address)) < 0) {
        perror("UDP Bind failed");
        close(serverM_UDP_PORT);
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < 3; i++) {
        memset(&udp_client_address[i], 0, sizeof(udp_client_address[i]));
        udp_client_address[i].sin_family = AF_INET;
        switch (i) {
        case 0:
            udp_client_address[i].sin_port = htons(serverA_UDP_PORT);
            break;
        case 1:
            udp_client_address[i].sin_port = htons(serverR_UDP_PORT);
            break;
        case 2:
            udp_client_address[i].sin_port = htons(serverD_UDP_PORT);
            break;
        }
    }

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

    char buffer[BUFFER_SIZE] = {0};
    while(1) {
        char username[100], password[100];
        memset(buffer, 0, BUFFER_SIZE);

        int tcp_client_socket = set_tcp_client_socket(tcp_server_socket);
        // printf("TCP client connected.\n");

        recv(tcp_client_socket, buffer, BUFFER_SIZE, 0);
        strcpy(username, buffer);
        // printf("client username: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);

        recv(tcp_client_socket, buffer, BUFFER_SIZE, 0);
        strcpy(password, buffer);
        // printf("client password: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);

        int authenticationCode;

        if(strcmp(username, "guest") == 0 && strcmp(password, "guest") == 0) {
            authenticationCode = 1;
            send(tcp_client_socket, &authenticationCode, sizeof(authenticationCode), 0);
        }
        else {
            printf("Server M has received username %s and password ****.\n", username);
            memset(buffer, 0, BUFFER_SIZE);

            sendto(udp_socket, username, strlen(username), 0, (struct sockaddr *)&udp_client_address[0], udp_client_len);
            sendto(udp_socket, password, strlen(password), 0, (struct sockaddr *)&udp_client_address[0], udp_client_len);
            printf("Server M has sent authentication request to Server A\n");

            recvfrom(udp_socket, &authenticationCode, sizeof(authenticationCode), 0, (struct sockaddr *)&udp_client_address[0], (socklen_t*)&udp_client_len);
            printf("The main server has received the response from server A using UDP over %d.\n", serverA_UDP_PORT);

            send(tcp_client_socket, &authenticationCode, sizeof(authenticationCode), 0);
            printf("The main server has sent the response from server A to client using TCP over port %d.\n", serverM_TCP_PORT);
        }

        if(authenticationCode == 0) continue;
        else if(authenticationCode == 1) {
            while(1) {
                int command_code = 0;
                char target[50];
                char lookup[] = "lookup";

                recv(tcp_client_socket, &command_code, sizeof(command_code), 0);

                recv(tcp_client_socket, buffer, BUFFER_SIZE, 0);
                strcpy(target, buffer);
                memset(buffer, 0, BUFFER_SIZE);

                if(command_code == 1) {
                    printf("The main server has received a lookup request from Guest to lookup %s’s repository using TCP over port %d.\n", target, serverM_TCP_PORT);
                    sendto(udp_socket, target, strlen(target), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    printf("The main server has sent the lookup request to server R.\n");

                    int n;
                    char **fileArr = malloc(n * sizeof(char*));
                    
                    recvfrom(udp_socket, &n, sizeof(n), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    printf("The main server has received the response from server R using UDP over %s\n", serverM_UDP_PORT);
                    for(int i = 0; i < n; i++) {
                        recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                        fileArr[i] = strdup(buffer);
                        memset(buffer, 0, BUFFER_SIZE);
                    }

                    send(tcp_client_socket, &n, sizeof(n), 0);
                    for(int i = 0; i < n; i++) {
                        usleep(50000);
                        send(tcp_client_socket, fileArr[i], sizeof(fileArr[i]), 0);
                        free(arr[i]);
                    }
                    printf("The main server has sent the response to the client.\n");
                }
            }
        }


        close(tcp_client_socket);
    }
    close(udp_socket);
    close(tcp_server_socket);
}