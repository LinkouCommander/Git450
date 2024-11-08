#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define UDP_PORT 8080
#define TCP_PORT 8180
#define BUFFER_SIZE 1024

// 初始化 UDP socket
int set_udp_socket() {
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(UDP_PORT);

    // 綁定 socket 到指定的地址和 port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("UDP Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// 初始化 TCP socket
int set_tcp_socket() {
    int server_fd;
    struct sockaddr_in address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("TCP Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(TCP_PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("TCP Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

// 接受 TCP 連線
int set_tcp_client_socket(int tcp_server_socket) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int client_socket;

    if ((client_socket = accept(tcp_server_socket, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("TCP Accept failed");
        exit(EXIT_FAILURE);
    }

    return client_socket;
}

int main() {
    // 初始化 UDP 和 TCP sockets
    int udp_socket = set_udp_socket();
    int tcp_server_socket = set_tcp_socket();

    char buffer[BUFFER_SIZE] = {0};
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    printf("Server is running...\n");

    while (1) {
        // 接受 TCP 客戶端的連接
        int tcp_client_socket = set_tcp_client_socket(tcp_server_socket);
        printf("TCP client connected.\n");

        // 接收 TCP 客戶端的訊息
        int n = recv(tcp_client_socket, buffer, BUFFER_SIZE, 0);
        if (n < 0) {
            perror("TCP Receive failed");
            close(tcp_client_socket);
            continue;
        }
        buffer[n] = '\0';
        printf("Message from TCP client: %s\n", buffer);

        // 回應 TCP 客戶端
        const char *tcp_response = "Hello from server";
        send(tcp_client_socket, tcp_response, strlen(tcp_response), 0);

        // 接收來自 UDP 客戶端的資料
        n = recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            perror("UDP Receive failed");
            continue;
        }
        buffer[n] = '\0';
        printf("Received message from UDP client: %s\n", buffer);

        // 回應 UDP 客戶端
        const char *udp_response = "Good morning my neighbors!";
        sendto(udp_socket, udp_response, strlen(udp_response), 0, (struct sockaddr *)&client_addr, client_len);
        printf("Response sent to UDP client\n");

        // 發送 UDP 客戶端資料給 TCP 客戶端
        send(tcp_client_socket, buffer, strlen(buffer), 0);

        close(tcp_client_socket);
    }

    // 關閉 socket
    close(udp_socket);
    close(tcp_server_socket);

    return 0;
}
