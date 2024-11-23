#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client.h"

void read_command(char *command, char *target) {
    char input[100];
    if(fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = '\0';
                
        if(sscanf(input, "%s %s", command, target) == 1) {
            strcpy(target, "");
        }
    }
    else {
        strcpy(command, "");
        strcpy(target, "");
    }
    printf("\n");
}

void lookup_op(int sock, const char *clientname, char *target) {
    char lookup_buffer[BUFFER_SIZE] = {0};
    int command_code = 1;

    if(strlen(target) == 0) {
        printf("Error: Username is required. Please specify a username to lookup.\n");
        return;
    }

    send(sock, &command_code, sizeof(command_code), 0);
    usleep(50000);
    send(sock, target, strlen(target), 0);
    printf("%s sent a lookup request to the main server.\n", clientname);

    int response_code;
    recv(sock, &response_code, sizeof(response_code), 0);
    printf("The client received the response from the main server using TCP over port %d\n\n", serverM_TCP_PORT);
    // printf("n = %d\n", n);

    if(response_code < 0) {
        printf("%s does not exist. Please try again.\n", target);
        return;
    }
    
    if(response_code == 0) {
        printf("Empty repository.\n");
        return;
    }
        
    int n = response_code;
    for(int i = 0; i < n; i++) {
        recv(sock, &lookup_buffer, BUFFER_SIZE, 0);
        printf("%s\n", lookup_buffer);
        memset(lookup_buffer, 0, BUFFER_SIZE);
    }
    // printf("...\n");
}

int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if(file) {
        fclose(file);
        return 1;
    }
    return 0;
}

void push_op(int sock, const char *clientname, const char *target) {
    char push_buffer[BUFFER_SIZE] = {0};
    int command_code = 2;

    if(strlen(target) == 0) {
        printf("Error: Filename is required. Please specify a filename to push.\n");
        return;
    }

    if(file_exists(target) == 0) {
        printf("Error: Invalid file: %s\n", target);
        return;
    }

    send(sock, &command_code, sizeof(command_code), 0);
    usleep(50000);
    send(sock, target, strlen(target), 0);

    int response_code;
    recv(sock, &response_code, sizeof(response_code), 0);
    if(!response_code) {
        printf("%s pushed successfully\n", target);
    } 
    else {
        printf("%s exists in %s's repository, do you want to overwrite (Y/N)?\n", target, clientname);
        
        char overwrite_input;
        while(1) {
            char input[100];
            if(fgets(input, sizeof(input), stdin)) {
                input[strcspn(input, "\n")] = '\0';
            }

            if(strlen(input) > 1) continue;
            overwrite_input = input[0];

            if(overwrite_input == 'y' || overwrite_input == 'Y' || overwrite_input == 'n'|| overwrite_input == 'N') break;
            else printf("Invalid input, please try again.\n");
        }

        int overwrite_code;
        if(overwrite_input == 'y' || overwrite_input == 'Y') {
            overwrite_code = 1;
            printf("%s pushed successfully.\n", target);
        }
        else {
            overwrite_code = 0;
            printf("%s was not pushed successfully.\n", target);
        }
        send(sock, &overwrite_code, sizeof(overwrite_code), 0);
    }
}

void deploy_op(int sock, const char *clientname) {
    char deploy_buffer[BUFFER_SIZE] = {0};
    int command_code = 3;

    send(sock, &command_code, sizeof(command_code), 0);
    usleep(50000);
    send(sock, "trash", strlen("trash"), 0);
    printf("%s sent a deploy request to the main server\n", clientname);

    int response_code;
    recv(sock, &response_code, sizeof(response_code), 0);
    printf("The client received the response from the main server using TCP over port %d. The following files in his/her repository have been deployed.\n\n", serverM_TCP_PORT);
        
    int n = response_code;
    for(int i = 0; i < n; i++) {
        recv(sock, &deploy_buffer, BUFFER_SIZE, 0);
        printf("%s\n", deploy_buffer);
        memset(deploy_buffer, 0, BUFFER_SIZE);
    }
}

void remove_op(int sock, const char *clientname, const char *target) {
    int command_code = 4;

    send(sock, &command_code, sizeof(command_code), 0);
    usleep(50000);
    send(sock, target, strlen(target), 0);
    printf("%s sent a remove request to the main server\n", clientname);

    int response_code;
    recv(sock, &response_code, sizeof(response_code), 0);
    if(!response_code) {
        printf("The remove request was successful.\n");
    }
    else {
        printf("The remove request failed.\n");
    }
}

void log_op(int sock, const char *clientname) {
    int command_code = 5;

    send(sock, &command_code, sizeof(command_code), 0);
    usleep(50000);
    send(sock, "trash", strlen("trash"), 0);
    printf("%s sent a log request to the main server\n", clientname);

    // int response_code;
    // recv(sock, &response_code, sizeof(response_code), 0);
}

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
    serv_addr.sin_port = htons(serverM_TCP_PORT);
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
            char command[50];
            char target[50];
            // char lookup[] = "lookup";
            memset(command, 0, 50);
            memset(target, 0, 50);

            printf("Please enter the command: <lookup <username>>\n");
            read_command(command, target);
            // printf("%s\n", command);
            // printf("%s\n", target);

            if(strcmp(command, "lookup") == 0) {
                lookup_op(sock, "Guest", target);
            }
            else {
                printf("Guests can only use the lookup command\n");
            }
            printf("\n---Start a new request---\n");
        }
    }
    else {
        printf("You have been granted member access.\n");

        while(1) {
            char command[50];
            char target[50];
            // char lookup[] = "lookup";
            memset(command, 0, 50);
            memset(target, 0, 50);

            printf("Please enter the command: \n<lookup <username>> \n<push <filename>> \n<remove <filename>> \n<deploy> \n<log>\n");
            read_command(command, target);
            // printf("%s\n", command);
            // printf("%s\n", target);

            if(strcmp(command, "lookup") == 0) {
                if(strlen(target) == 0) {
                    strcpy(target, username);
                    printf("Username is not specified. Will lookup %s.\n", username);
                }
                lookup_op(sock, username, target);
            }
            else if(strcmp(command, "push") == 0) {
                push_op(sock, username, target);
            }
            else if(strcmp(command, "deploy") == 0) {
                deploy_op(sock, username);
            }
            else if(strcmp(command, "remove") == 0) {
                remove_op(sock, username, target);
            }
            else if(strcmp(command, "log") == 0) {
                log_op(sock, username);
            }
            else {
                printf("Wrong command\n");
            }
            printf("\n---Start a new request---\n");
        }
    }
    

    // 關閉連接
    close(sock);
    return 0;
}
