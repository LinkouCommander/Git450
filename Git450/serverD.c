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
#include "serverD.h"

void file_write(const char* username, const char* filename) {
    FILE* push_file = fopen("deployed.txt", "a");
    if(!push_file) {
        perror("Can't open deployed.txt");
        exit(EXIT_FAILURE);
    }
    fprintf(push_file, "%s %s\n", username, filename);
    // printf("%s %s\n", username, filename);

    fclose(push_file);
}

int set_udp_socket() {
    int sockfd;
    struct sockaddr_in address;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(serverR_UDP_PORT);
    
    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Socket Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int main() {
    FILE* file = fopen("deployed.txt", "a");
    if(!file) {
        perror("Can't open deployed.txt");
        exit(EXIT_FAILURE);
    }
    // fprintf(file, "UserName Filename\n");

    fclose(file);

    // setup socket
    int serverD_socket = set_udp_socket();
    struct sockaddr_in address;
    int addr_len = sizeof(address);

    printf("Server D is up and running using UDP on port %d.\n", serverR_UDP_PORT);

    char buffer[BUFFER_SIZE] = {0};
    char username[100];
    while(1) {
        memset(buffer, 0, BUFFER_SIZE);
        memset(username, 0, 100);

        int n;
        recvfrom(serverD_socket, &n, sizeof(n), 0, (struct sockaddr*)&address, &addr_len);
        recvfrom(serverD_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&address, &addr_len);
        strcpy(username, buffer);
        memset(buffer, 0, BUFFER_SIZE);
        
        char **fileArr = malloc(n * sizeof(char*));
        for(int i = 0; i < n; i++) {
            recvfrom(serverD_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&address, &addr_len);
            fileArr[i] = strdup(buffer);
            memset(buffer, 0, BUFFER_SIZE);
            // printf("%s\n", fileArr[i]);
        }
        printf("Server D has received a deploy request from the main server.\n");

        for(int i = 0; i < n; i++) {
            file_write(username, fileArr[i]);
            free(fileArr[i]);
        }
        free(fileArr);
        
        printf("Server D has deployed the user %s's repository.\n", username);
        sendto(serverD_socket, &n, sizeof(n), 0, (struct sockaddr*)&address, addr_len);
    }
}