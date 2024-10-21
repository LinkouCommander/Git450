#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_len = sizeof(client_addr);

    // 創建 socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    // 清空 server_addr 結構
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // 設置 server address
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 綁定 socket 到指定的地址和 port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(sockfd);
        return -1;
    }

    std::cout << "Server is waiting for messages..." << std::endl;

    // 循環接收來自 client 的數據
    while (true) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        buffer[n] = '\0'; // 將接收的數據轉換為字串
        std::cout << "Received message: " << buffer << std::endl;

        // 發送回應給 client
        const char *response = "Hello from server";
        sendto(sockfd, response, strlen(response), 0, (const struct sockaddr *)&client_addr, client_len);
        std::cout << "Response sent to client" << std::endl;
    }

    close(sockfd);
    return 0;
}
