#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    const char *message = "Hello from client";

    // 創建 socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    // 設置 server 地址
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 發送訊息給 server
    sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    std::cout << "Message sent to server" << std::endl;

    // 接收來自 server 的回應
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
    buffer[n] = '\0'; // 將接收的數據轉換為字串
    std::cout << "Response from server: " << buffer << std::endl;

    close(sockfd);
    return 0;
}
