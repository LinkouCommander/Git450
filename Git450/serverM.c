#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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

    if (bind(udp_socket, (const struct sockaddr *)&udp_server_address, sizeof(udp_server_address)) < 0) {
        perror("UDP Bind failed");
        close(udp_socket);
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
    // struct sockaddr_in udp_server_address, udp_client_address;
    // socklen_t udp_client_len = sizeof(udp_client_address);

    // memset(&udp_server_address, 0, sizeof(udp_server_address));
    // memset(&udp_client_address, 0, sizeof(udp_client_address));

    // udp_server_address.sin_family = AF_INET; // IPv4
    // udp_server_address.sin_addr.s_addr = INADDR_ANY;
    // udp_server_address.sin_port = htons(serverA_UDP_PORT);

    // if (bind(udp_socket, (const struct sockaddr *)&udp_server_address, sizeof(udp_server_address)) < 0) {
    //     perror("UDP Bind failed");
    //     close(udp_socket);
    //     exit(EXIT_FAILURE);
    // }

    // const char *udp_message = "Greetings from serverM!";

    // sendto(udp_socket, udp_message, strlen(udp_message), 0, (const struct sockaddr *)&udp_client_address, sizeof(udp_client_address));
    printf("Message sent to serverA\n");

    int tcp_server_socket = set_tcp_socket();
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    printf("Server is running...\n");

    while(1) {
        char buffer[BUFFER_SIZE] = {0};
        char username[100], password[100];
        memset(buffer, 0, BUFFER_SIZE);

        int tcp_client_socket = set_tcp_client_socket(tcp_server_socket);
        printf("TCP client connected.\n");

        if(recv(tcp_client_socket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("TCP Receive failed");
            exit(EXIT_FAILURE);
        }
        strcpy(username, buffer);
        printf("client username: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);

        if(recv(tcp_client_socket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("TCP Receive failed");
            exit(EXIT_FAILURE);
        }
        strcpy(password, buffer);
        printf("client password: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);

        if(strcmp(username, "guest") == 0 && strcmp(password, "guest") == 0) {
            printf("Guest\n");
            send(tcp_client_socket, "Guest Mode", strlen("Guest Mode"), 0);
        }
        else {
            printf("Not Guest\n");
            // recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&udp_client_address, &udp_client_len);
            memset(buffer, 0, BUFFER_SIZE);

            sendto(udp_socket, username, strlen(username), 0, (struct sockaddr *)&udp_client_address, udp_client_len);
            sendto(udp_socket, password, strlen(password), 0, (struct sockaddr *)&udp_client_address, udp_client_len);
            printf("Response sent to UDP client\n");
            // // send(tcp_client_socket, udp_response, strlen(udp_response), 0);
            // // printf("Response sent to TCP client\n");

            int authenticationCode;
            recvfrom(udp_socket, &authenticationCode, sizeof(authenticationCode), 0, (struct sockaddr *)&udp_client_address, (socklen_t*)&udp_client_len);
            printf("Authentication Code: %d\n", authenticationCode);
            char authentication_message[100];
            if(authenticationCode) {
                sprintf(authentication_message, "Member %s has been authenticated\n", username);
            }
            else {
                sprintf(authentication_message, "The username %s or password ****** is incorrect\n", username);
            }
            send(tcp_client_socket, authentication_message, strlen(authentication_message), 0);
        }
        // const char *tcp_response = "Hello TCP\n";
        // send(tcp_client_socket, tcp_response, strlen(tcp_response), 0);
        close(tcp_client_socket);
    }
    close(udp_socket);
    close(tcp_server_socket);
}