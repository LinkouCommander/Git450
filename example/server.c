#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // 創建 socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // 綁定地址和端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    
    // 開始監聽
    listen(server_fd, 3);
    
    // 接受連接
    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    
    // 接收資料
    recv(new_socket, buffer, BUFFER_SIZE, 0);
    printf("Message from client: %s\n", buffer);
    
    // 發送回應
    send(new_socket, "Hello from server", strlen("Hello from server"), 0);
    
    // 關閉連接
    close(new_socket);
    close(server_fd);
    return 0;
}
