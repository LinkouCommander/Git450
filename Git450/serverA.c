#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8180
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    const char *message = "Good morning my neighbors!";

    char username[100];
    char password[100];

    // 創建 socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 設置 server 地址
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 發送訊息給 server
    // sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    // printf("Message sent to server\n");

    // 接收來自 server 的回應
    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
    strcpy(username, buffer);

    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
    strcpy(password, buffer);

    // buffer[n] = '\0'; // 將接收的數據轉換為字串
    printf("Username: %s\n Password: %s", username, password);

    close(sockfd);
    return 0;
}
