#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "serverM.h"

Command* add_command(Command* logs, int* size, const char* username, const char* command, const char* target) {
    logs = realloc(logs, (*size + 1) * sizeof(Command));
    if (logs == NULL) {
        perror("Failed to reallocate memory");
        return NULL;
    }

    strcpy(logs[*size].username, username);
    strcpy(logs[*size].command, command);
    if(strcmp(target, "trash") == 0) strcpy(logs[*size].target, "");
    else strcpy(logs[*size].target, target);

    (*size)++;

    return logs;
}

int set_udp_socket() {
    int sockfd;
    struct sockaddr_in udp_server_address;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&udp_server_address, 0, sizeof(udp_server_address));

    udp_server_address.sin_family = AF_INET; // IPv4
    udp_server_address.sin_addr.s_addr = INADDR_ANY;
    udp_server_address.sin_port = htons(serverM_UDP_PORT);

    if (bind(sockfd, (const struct sockaddr *)&udp_server_address, sizeof(udp_server_address)) < 0) {
        perror("UDP Bind failed");
        close(serverM_UDP_PORT);
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < 3; i++) {
        memset(&udp_client_address[i], 0, sizeof(udp_client_address[i]));
        udp_client_address[i].sin_family = AF_INET;
        switch (i) {
        case 0:
            udp_client_address[i].sin_port = htons(serverA_UDP_PORT);
            break;
        case 1:
            udp_client_address[i].sin_port = htons(serverR_UDP_PORT);
            break;
        case 2:
            udp_client_address[i].sin_port = htons(serverD_UDP_PORT);
            break;
        }
    }

    return sockfd;
}

int set_tcp_socket() {
    int sockfd;
    struct sockaddr_in address;
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("TCP Socket creation failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(serverM_TCP_PORT);

    if (bind(sockfd, (const struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("TCP Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if(listen(sockfd, 3) < 0) {
        perror("Listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int set_tcp_client_socket(int tcp_server_socket) {
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    int sockfd;
    if ((sockfd = accept(tcp_server_socket, (struct sockaddr*)&address, (socklen_t*)&addr_len)) < 0) {
        perror("TCP Accept failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int main() {
    int cur_size = 0;
    Command *logs = malloc(cur_size * sizeof(Command));
    if (logs == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }
    

    int udp_socket = set_udp_socket();

    int tcp_server_socket = set_tcp_socket();
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    printf("Server M is up and running using UDP on port %d.\n", serverM_UDP_PORT);

    char buffer[BUFFER_SIZE] = {0};
    while(1) {
        char username[100], password[100];
        memset(buffer, 0, BUFFER_SIZE);

        int tcp_client_socket = set_tcp_client_socket(tcp_server_socket);
        // printf("TCP client connected.\n");

        recv(tcp_client_socket, buffer, BUFFER_SIZE, 0);
        strcpy(username, buffer);
        // printf("client username: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);

        recv(tcp_client_socket, buffer, BUFFER_SIZE, 0);
        strcpy(password, buffer);
        // printf("client password: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);

        int authenticationCode;

        if(strcmp(username, "guest") == 0 && strcmp(password, "guest") == 0) {
            authenticationCode = 1;
            send(tcp_client_socket, &authenticationCode, sizeof(authenticationCode), 0);
        }
        else {
            printf("Server M has received username %s and password ****.\n", username);
            memset(buffer, 0, BUFFER_SIZE);

            sendto(udp_socket, username, strlen(username), 0, (struct sockaddr *)&udp_client_address[0], udp_client_len);
            sendto(udp_socket, password, strlen(password), 0, (struct sockaddr *)&udp_client_address[0], udp_client_len);
            printf("Server M has sent authentication request to Server A\n");

            recvfrom(udp_socket, &authenticationCode, sizeof(authenticationCode), 0, (struct sockaddr *)&udp_client_address[0], (socklen_t*)&udp_client_len);
            printf("The main server has received the response from server A using UDP over %d.\n", serverM_UDP_PORT);

            send(tcp_client_socket, &authenticationCode, sizeof(authenticationCode), 0);
            printf("The main server has sent the response from server A to client using TCP over port %d.\n", serverM_TCP_PORT);
        }

        if(authenticationCode == 0) continue;
        else if(authenticationCode == 1) {
            while(1) {
                int command_code = 0;
                char target[50];

                int debug_code;
                if (debug_code = recv(tcp_client_socket, &command_code, sizeof(command_code), 0) <= 0) {
                    break;
                }

                recv(tcp_client_socket, buffer, BUFFER_SIZE, 0);
                strcpy(target, buffer);
                memset(buffer, 0, BUFFER_SIZE);

                if(command_code == 1) {
                    printf("The main server has received a lookup request from Guest to lookup %s's repository using TCP over port %d.\n", target, serverM_TCP_PORT);
                    
                    sendto(udp_socket, &command_code, sizeof(command_code), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    usleep(50000);
                    sendto(udp_socket, target, strlen(target), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    printf("The main server has sent the lookup request to server R.\n");

                    int n;
                    recvfrom(udp_socket, &n, sizeof(n), 0, (struct sockaddr *)&udp_client_address[1], &udp_client_len);
                    printf("The main server has received the response from server R using UDP over %d\n", serverM_UDP_PORT);
                    
                    char **fileArr = malloc(n * sizeof(char*));
                    for(int i = 0; i < n; i++) {
                        recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&udp_client_address[1], &udp_client_len);
                        fileArr[i] = strdup(buffer);
                        memset(buffer, 0, BUFFER_SIZE);
                        // printf("%s\n", fileArr[i]);
                    }
                    
                    send(tcp_client_socket, &n, sizeof(n), 0);
                    for(int i = 0; i < n; i++) {
                        usleep(50000);
                        send(tcp_client_socket, fileArr[i], strlen(fileArr[i]), 0);
                        free(fileArr[i]);
                    }
                    printf("The main server has sent the response to the client.\n");
                }
            }
        }
        else {
            while(1) {
                int command_code = 0;
                char target[50];
                const char *commands[] = {"", "lookup", "push", "deploy", "remove", "log"};

                int debug_code;
                if ((debug_code = recv(tcp_client_socket, &command_code, sizeof(command_code), 0)) <= 0) {
                    break;
                }

                recv(tcp_client_socket, buffer, BUFFER_SIZE, 0);
                strcpy(target, buffer);
                memset(buffer, 0, BUFFER_SIZE);

                logs = add_command(logs, &cur_size, username, commands[command_code], target);

                if(command_code == 1) {
                    printf("The main server has received a lookup request from %s to lookup %s's repository using TCP over port %d.\n", username, target, serverM_TCP_PORT);
                    
                    sendto(udp_socket, &command_code, sizeof(command_code), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    usleep(50000);
                    sendto(udp_socket, target, strlen(target), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    printf("The main server has sent the lookup request to server R.\n");
                    
                    int response_code;
                    recvfrom(udp_socket, &response_code, sizeof(response_code), 0, (struct sockaddr *)&udp_client_address[1], &udp_client_len);
                    printf("The main server has received the response from server R using UDP over %d\n", serverM_UDP_PORT);
                    
                    if(response_code == -1 || response_code == 0) {
                        send(tcp_client_socket, &response_code, sizeof(response_code), 0);
                        continue;
                    }
                    
                    int n = response_code;
                    char **fileArr = malloc(n * sizeof(char*));
                    for(int i = 0; i < n; i++) {
                        recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&udp_client_address[1], &udp_client_len);
                        fileArr[i] = strdup(buffer);
                        memset(buffer, 0, BUFFER_SIZE);
                        // printf("%s\n", fileArr[i]);
                    }
                    
                    send(tcp_client_socket, &n, sizeof(n), 0);
                    for(int i = 0; i < n; i++) {
                        usleep(50000);
                        send(tcp_client_socket, fileArr[i], strlen(fileArr[i]), 0);
                        free(fileArr[i]);
                    }
                    free(fileArr);
                    printf("The main server has sent the response to the client.\n");
                }
                else if(command_code == 2) {
                    printf("The main server has received a push request from %s, using TCP over port %d.\n", username, serverM_TCP_PORT);

                    sendto(udp_socket, &command_code, sizeof(command_code), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    usleep(50000);
                    sendto(udp_socket, username, strlen(username), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    usleep(50000);
                    sendto(udp_socket, target, strlen(target), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    printf("The main server has sent the push request to server R.\n");

                    int response_code;
                    recvfrom(udp_socket, &response_code, sizeof(response_code), 0, (struct sockaddr *)&udp_client_address[1], &udp_client_len);
                    if(!response_code) {
                        printf("The main server has received the response from server R using UDP over %d\n", serverM_UDP_PORT);
                        send(tcp_client_socket, &response_code, sizeof(response_code), 0);
                        printf("The main server has sent the response to the client.\n");
                    }
                    else {
                        printf("The main server has received the response from server R using UDP over %d, asking for overwrite confirmation\n", serverM_UDP_PORT);
                        send(tcp_client_socket, &response_code, sizeof(response_code), 0);
                        printf("The main server has sent the overwrite confirmation request to the client.\n");

                        int overwrite_code;
                        if((debug_code = recv(tcp_client_socket, &overwrite_code, sizeof(overwrite_code), 0)) <= 0) {
                            int fail_code = -1;
                            sendto(udp_socket, &fail_code, sizeof(fail_code), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                            break;
                        }
                        printf("The main server has received the overwrite confirmation response from %s using TCP over port %d\n", username, serverM_TCP_PORT);

                        sendto(udp_socket, &overwrite_code, sizeof(overwrite_code), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                        printf("The main server has sent the overwrite confirmation response to server R.\n");
                    }
                }
                else if(command_code == 3) {
                    printf("The main server has received a deploy request from %s, using TCP over port %d.\n", username, serverM_TCP_PORT);
                    
                    sendto(udp_socket, &command_code, sizeof(command_code), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    usleep(50000);
                    sendto(udp_socket, username, strlen(username), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    printf("The main server has sent the deploy request to server R.\n");

                    int n;
                    recvfrom(udp_socket, &n, sizeof(n), 0, (struct sockaddr *)&udp_client_address[1], &udp_client_len);
                    
                    char **fileArr = malloc(n * sizeof(char*));
                    for(int i = 0; i < n; i++) {
                        recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&udp_client_address[1], &udp_client_len);
                        fileArr[i] = strdup(buffer);
                        memset(buffer, 0, BUFFER_SIZE);
                        // printf("%s\n", fileArr[i]);
                    }
                    printf("The main server received the deploy response from server R.\n");
                    
                    sendto(udp_socket, &n, sizeof(n), 0, (struct sockaddr *)&udp_client_address[2], udp_client_len);
                    usleep(50000);
                    sendto(udp_socket, username, strlen(username), 0, (struct sockaddr *)&udp_client_address[2], udp_client_len);
                    for(int i = 0; i < n; i++) {
                        usleep(50000);
                        sendto(udp_socket, fileArr[i], strlen(fileArr[i]), 0, (struct sockaddr *)&udp_client_address[2], udp_client_len);
                    }
                    printf("The main server has sent the deploy request to server D.\n");

                    int ack;
                    recvfrom(udp_socket, &ack, sizeof(ack), 0, (struct sockaddr *)&udp_client_address[2], &udp_client_len);
                    printf("The user %s's repository has been deployed at server D.\n", username);

                    send(tcp_client_socket, &n, sizeof(n), 0);
                    for(int i = 0; i < n; i++) {
                        usleep(50000);
                        send(tcp_client_socket, fileArr[i], strlen(fileArr[i]), 0);
                        free(fileArr[i]);
                    }
                    free(fileArr);
                }
                else if(command_code == 4) {
                    printf("The main server has received a remove request from %s, using TCP over port %d.\n", username, serverM_TCP_PORT);
                    
                    sendto(udp_socket, &command_code, sizeof(command_code), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    usleep(50000);
                    sendto(udp_socket, username, strlen(username), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);
                    usleep(50000);
                    sendto(udp_socket, target, strlen(target), 0, (struct sockaddr *)&udp_client_address[1], udp_client_len);

                    int response_code;
                    recvfrom(udp_socket, &response_code, sizeof(response_code), 0, (struct sockaddr *)&udp_client_address[1], &udp_client_len);
                    printf("The main server has received confirmation of the remove request done by the server R.\n");
                    send(tcp_client_socket, &response_code, sizeof(response_code), 0);
                }
                else if(command_code == 5) {
                    printf("The main server has received a log request from %s, using TCP over port %d.\n", username, serverM_TCP_PORT);
                    
                    int log_code = 0;
                    Command *historyLog = malloc(log_code * sizeof(Command));
                    if(!historyLog) {
                        perror("Failed to allocate memory");
                        return 1;
                    }

                    for(int i = 0; i < cur_size; i++) {
                        if(strcmp(logs[i].username, username) == 0) {
                            historyLog = add_command(historyLog, &log_code, username, logs[i].command, logs[i].target);
                            // printf("%d. %s %s\n", log_code, logs[i].command, logs[i].target);
                        }
                    }

                    send(tcp_client_socket, &log_code, sizeof(log_code), 0);
                    for(int i = 0; i < log_code; i++) {
                        usleep(50000);
                        sprintf(buffer, "%s %s", historyLog[i].command, historyLog[i].target);
                        send(tcp_client_socket, buffer, strlen(buffer), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                    }
                    printf("The main server has sent the log response to the client.\n");
                    
                    free(historyLog);
                }
            }
        }


        close(tcp_client_socket);
    }
    free(logs);

    close(udp_socket);
    close(tcp_server_socket);
}