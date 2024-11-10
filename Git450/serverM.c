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
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
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
    if (listen(sockfd, 3) < 0) {
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
    if (sockfd = accept(tcp_server_socket, (struct sockaddr*)&address, &addr_len) < 0) {
        perror("TCP Accept failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int main() {
    int udp_socket = set_udp_socket();
    struct sockaddr_in udp_address;
    memset(&udp_address, 0, sizeof(udp_address));
    udp_address.sin_family = AF_INET;
    udp_address.sin_addr.s_addr = INADDR_ANY;
    udp_address.sin_port = htons(UDP_PORT);

    if (bind(udp_socket, (const struct sockaddr *)&udp_address, sizeof(udp_address)) < 0) {
        perror("UDP Bind failed");
        close(udp_socket);
        exit(EXIT_FAILURE);
    }

    int tcp_server_socket = set_tcp_socket();
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    printf("Server is running...\n");

    while (1) {
        char buffer[BUFFER_SIZE];
        char username[100], password[100];
        memset(buffer, 0, BUFFER_SIZE);

        int tcp_client_socket = set_tcp_client_socket(tcp_server_socket);
        printf("TCP client connected.\n");

        // 接收 username
        recv(tcp_client_socket, buffer, BUFFER_SIZE, 0);
        strcpy(username, buffer);
        printf("Received username: %s\n", username);
        memset(buffer, 0, BUFFER_SIZE);

        // 接收 password
        recv(tcp_client_socket, buffer, BUFFER_SIZE, 0);
        strcpy(password, buffer);
        printf("Received password: %s\n", password);
        memset(buffer, 0, BUFFER_SIZE);

        // 等待 UDP Client 發送初始訊息以獲取其地址
        // recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        // memset(buffer, 0, BUFFER_SIZE);

        // 驗證並回覆 UDP Client
        if (strcmp(username, "guest") != 0 && strcmp(password, "guest") != 0) {
            printf("Member?\n");
            sendto(udp_socket, username, strlen(username), 0, (struct sockaddr *)&client_addr, client_len);
            sendto(udp_socket, password, strlen(password), 0, (struct sockaddr *)&client_addr, client_len);
            printf("Sent response to UDP client.\n");
        } 
        else {
            printf("Guest.\n");
        }
        close(tcp_client_socket);
    }

    close(udp_socket);
    close(tcp_server_socket);
    return 0;
}
