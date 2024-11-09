#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define UDP_PORT 8080
#define TCP_PORT 8180
#define BUFFER_SIZE 1024

// 設置 UDP socket
int set_udp_socket() {
    int sockfd;
    struct sockaddr_in address;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(UDP_PORT);

    if (bind(sockfd, (const struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("UDP Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// 設置 TCP socket
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

// 接受 TCP 連線
int set_tcp_client_socket(int tcp_server_socket) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_socket = accept(tcp_server_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (client_socket < 0) {
        perror("TCP Accept failed");
        exit(EXIT_FAILURE);
    }
    return client_socket;
}

int main() {
    int udp_socket = set_udp_socket();
    int tcp_server_socket = set_tcp_socket();

    struct sockaddr_in udp_client_addr;
    socklen_t udp_client_len = sizeof(udp_client_addr);
    char buffer[BUFFER_SIZE] = {0};

    printf("Server is running...\n");

    while (1) {
        // 等待 TCP 客戶端連線
        int tcp_client_socket = set_tcp_client_socket(tcp_server_socket);
        printf("TCP client connected.\n");

        // 接收 TCP 客戶端的初始訊息
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(tcp_client_socket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("TCP Receive failed");
            close(tcp_client_socket);
            continue;
        }
        printf("Message from TCP client: %s\n", buffer);

        const char *tcp_response = "Hello from server\n";
        send(tcp_client_socket, tcp_response, strlen(tcp_response), 0);

        // 等待 UDP 訊息
        memset(buffer, 0, BUFFER_SIZE);
        printf("Waiting for UDP message...\n");

        if (recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&udp_client_addr, &udp_client_len) < 0) {
            perror("UDP Receive failed");
            close(tcp_client_socket);
            continue;
        }

        // 收到 UDP 訊息後回應 TCP 客戶端
        printf("Received message from UDP client: %s\n", buffer);

        const char *udp_response = "Message received via UDP\n";
        sendto(udp_socket, udp_response, strlen(udp_response), 0, (struct sockaddr *)&udp_client_addr, udp_client_len);
        printf("Response sent to UDP client\n");

        send(tcp_client_socket, udp_response, strlen(udp_response), 0);
        printf("Response sent to TCP client\n");

        // 關閉 TCP 連線
        close(tcp_client_socket);
    }

    close(udp_socket);
    close(tcp_server_socket);
    return 0;
}
