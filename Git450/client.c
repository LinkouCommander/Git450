#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 25048
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    // printf("%s\n", argv[1]);
    // printf("%s\n", argv[2]);
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
    const char* username = " ";
    const char* password = " ";

    if(argc > 1) username = argv[1];
    // printf("%s\n", username);
    if(argc > 2) password = argv[2];
    // printf("%s\n", password);
    send(sock, username, strlen(username), 0);
    usleep(50000);
    send(sock, password, strlen(password), 0);

    int authenticationCode;
    recv(sock, &authenticationCode, sizeof(authenticationCode), 0);
    if(!authenticationCode) {
        printf("The credentials are incorrect. Please try again.\n");
        exit(EXIT_FAILURE);
    }
    else if(authenticationCode == 1) {
        printf("You have been granted guest access.\n");

        while(1) {
            printf("Please enter the command: <lookup <username>>\n");

            int command_code = 0;
            char command[50];
            char target[50];
            char lookup[] = "lookup";
            scanf("%s %s", command, target);

            if(strcmp(command, lookup) == 0) {
                command_code = 1;
                if(strlen(target) == 0) {
                    printf("Error: Username is required. Please specify a username to lookup.\n");
                }
                else {
                    send(sock, &command_code, sizeof(command_code), 0);
                    usleep(50000);
                    send(sock, target, strlen(target), 0);
                    printf("Guest sent a lookup request to the main server.\n");

                    int n;
                    recv(sock, &n, sizeof(n), 0);
                    printf("The client received the response from the main server using TCP over port %s\n", PORT);
                    for(int i = 0; i < n; i++) {
                        recv(sock, &buffer, BUFFER_SIZE, 0);
                        printf("%s\n", buffer);
                        memset(buffer, 0, BUFFER_SIZE);
                    }
                }
            }
            else {
                printf("Guests can only use the lookup command\n");
            }

            printf("—--Start a new request—--\n");
        }
    }
    else {
        printf("You have been granted member access.\n");
    }
    

    // 關閉連接
    close(sock);
    return 0;
}
