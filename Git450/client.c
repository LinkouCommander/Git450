#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 25048
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
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

    printf("The client is up and running.\n");
    
    // 發送資料
    const char* username = "";
    const char* password = "";

    if(argc > 1) username = argv[1];
    if(argc > 2) password = argv[2];
    // printf("username: ");
    // scanf("%s", username);

    send(sock, username, strlen(username), 0);
    
    // printf("password: ");
    // scanf("%s", password);

    send(sock, password, strlen(password), 0);

    int authenticationCode;
    recv(sock, &authenticationCode, sizeof(authenticationCode), 0);
    // printf("%s\n", buffer);
    if(!authenticationCode) {
        printf("The credentials are incorrect. Please try again.\n");
    }
    else if(authenticationCode == 1) {
        printf("You have been granted guest access.\n");
    }
    else {
        printf("You have been granted member access.\n");
    }
    
    // 關閉連接
    close(sock);
    return 0;
}
