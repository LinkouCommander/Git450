#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define UDP_PORT 8080
#define BUFFER_SIZE 1024

int set_udp_socket() {
    int sockfd;
    struct sockaddr_in address;

    // 建立 UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int main() {
    int serverA_socket = set_udp_socket();
    struct sockaddr_in server_addr;

    // 設置伺服器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 伺服器 IP

    const char *message = "Good morning my neighbors!";
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);

    // 發送訊息給伺服器
    if (sendto(serverA_socket, message, strlen(message), 0, 
               (const struct sockaddr *)&server_addr, addr_len) < 0) {
        perror("Failed to send message");
        close(serverA_socket);
        exit(EXIT_FAILURE);
    }
    printf("Message sent to server: %s\n", message);

    // 接收來自伺服器的回應
    int n = recvfrom(serverA_socket, buffer, BUFFER_SIZE, 0, 
                     (struct sockaddr *)&server_addr, &addr_len);
    if (n < 0) {
        perror("Failed to receive message");
        close(serverA_socket);
        exit(EXIT_FAILURE);
    }

    buffer[n] = '\0'; // 將接收的數據轉換為字串
    printf("Received response from server: %s\n", buffer);

    close(serverA_socket);
    return 0;
}
