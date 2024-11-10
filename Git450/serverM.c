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

    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
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
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int sockfd = accept(tcp_server_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (sockfd < 0) {
        perror("TCP Accept failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int main() {
    int udp_socket = set_udp_socket();
    int tcp_server_socket = set_tcp_socket();
    struct sockaddr_in udp_client_addr;
    socklen_t client_len = sizeof(udp_client_addr);

    memset(&udp_client_addr, 0, sizeof(udp_client_addr));
    udp_client_addr.sin_family = AF_INET;
    udp_client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    udp_client_addr.sin_port = htons(UDP_PORT);

    printf("Server is running...\n");

    while (1) {
        char buffer[BUFFER_SIZE] = {0};
        char username[100], password[100];

        // 接收 TCP Client 連線
        int tcp_client_socket = set_tcp_client_socket(tcp_server_socket);
        printf("TCP client connected.\n");

        // 接收使用者名稱
        if (recv(tcp_client_socket, username, sizeof(username), 0) < 0) {
            perror("TCP Receive failed (username)");
            close(tcp_client_socket);
            continue;
        }
        printf("Client username: %s\n", username);

        // 接收密碼
        if (recv(tcp_client_socket, password, sizeof(password), 0) < 0) {
            perror("TCP Receive failed (password)");
            close(tcp_client_socket);
            continue;
        }
        printf("Client password: %s\n", password);

        // 如果不是 "guest"，透過 UDP 回傳帳號與密碼
        if (strcmp(username, "guest") != 0 || strcmp(password, "guest") != 0) {
            sendto(udp_socket, username, strlen(username), 0, (struct sockaddr *)&udp_client_addr, client_len);
            sendto(udp_socket, password, strlen(password), 0, (struct sockaddr *)&udp_client_addr, client_len);
            printf("Response sent to UDP client\n");
        } else {
            printf("Guest login detected\n");
        }

        const char *tcp_response = "Hello TCP\n";
        send(tcp_client_socket, tcp_response, strlen(tcp_response), 0);
        close(tcp_client_socket);
    }

    close(udp_socket);
    close(tcp_server_socket);
    return 0;
}
