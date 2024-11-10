#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define UDP_PORT 8080
#define TCP_PORT 8180
#define BUFFER_SIZE 1024

int set_udp_socket() {
    int sockfd;
    struct sockaddr_in address;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(UDP_PORT);

    // 綁定 socket 到指定的地址和 port
    if (bind(sockfd, (const struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("UDP Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
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
    address.sin_port = htons(TCP_PORT);

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
    int tcp_server_socket = set_tcp_socket();

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};

    printf("Server is running...\n");

    while(1) {
        char username[100];
        char password[100];
        int tcp_client_socket = set_tcp_client_socket(tcp_server_socket);
        printf("TCP client connected.\n");

        if(recv(tcp_client_socket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("TCP Receive failed");
            exit(EXIT_FAILURE);
        }
        strcpy(username, buffer);
        printf("client username: %s\n", buffer);

        if(recv(tcp_client_socket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("TCP Receive failed");
            exit(EXIT_FAILURE);
        }
        strcpy(password, buffer);
        printf("client password: %s\n", buffer);

        if(strcmp(username, "guest") != 0 && strcmp(password, "guest") != 0) {
            if(recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len) < 0) {
                perror("UDP Receive failed\n");
                exit(EXIT_FAILURE);
            }
            memset(buffer, '\0', sizeof(buffer));
            printf("Received message from UDP client: %s\n", buffer);

            const char *udp_response = "Hello UDP\n";
            sendto(udp_socket, username, strlen(username), 0, (const struct sockaddr *)&client_addr, client_len);
            sendto(udp_socket, password, strlen(password), 0, (const struct sockaddr *)&client_addr, client_len);
            printf("Response sent to UDP client\n");
            // send(tcp_client_socket, udp_response, strlen(udp_response), 0);
            // printf("Response sent to TCP client\n");
        }

        const char *tcp_response = "Hello TCP\n";
        send(tcp_client_socket, tcp_response, strlen(tcp_response), 0);


        close(tcp_client_socket);
    }
    close(udp_socket);
    close(tcp_server_socket);
}