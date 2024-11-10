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

int main() {
    int serverA_socket = set_udp_socket();
    struct sockaddr_in address;
    // socklen_t addr_len = sizeof(address);

    const char *message = "Good morning my neighbors!";

    while(1) {
        char buffer[BUFFER_SIZE];

        char username[100];
        char password[100];

        // 發送訊息給 server
        sendto(serverA_socket, message, strlen(message), 0, (const struct sockaddr *)&address, sizeof(address));
        printf("Message sent to server\n");

        // // 接收來自 server 的回應
        // recvfrom(serverA_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
        // strcpy(username, buffer);

        // recvfrom(serverA_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
        // strcpy(password, buffer);

        // buffer[n] = '\0'; // 將接收的數據轉換為字串
        // printf("Username: %s\n Password: %s", username, password);
    }

    close(serverA_socket);
    return 0;
}
