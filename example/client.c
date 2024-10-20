#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    
    // 創建 socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    // 設置 server 地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    // 連接 server
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    // 發送資料
    send(sock, "Hello from client", strlen("Hello from client"), 0);
    
    // 接收 server 的回應
    recv(sock, buffer, BUFFER_SIZE, 0);
    printf("Message from server: %s\n", buffer);
    
    // 關閉連接
    close(sock);
    return 0;
}
