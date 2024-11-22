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
#include "serverR.h"

#define TABLE_SIZE 2001

UserFile *read_file(UserFile *userfiles, int *size, const char *username, const char *filename) {
    userfiles = realloc(userfiles, (*size + 1) * sizeof(userfiles));
    if(!userfiles) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    strcpy(userfiles[*size].username, username);
    strcpy(userfiles[*size].filename, filename);
    (*size)++;

    return userfiles;
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
    char buffer[BUFFER_SIZE] = {0};

    // get file info
    FILE *file = fopen("filenames.txt", "r");
    if(file == NULL) {
        perror("Can't open filenames.txt");
        return 1;
    }

    char username[100];
    char filename[100];
    
    char row[1];
    fgets(row, sizeof(row), file);

    int size = 0;
    UserFile* fileInfo = NULL;
    while(fscanf(file, "%s %s", username, filename) != EOF) {
        fileInfo = read_file(fileInfo, &size, username, filename);
    }

    fclose(file);

    // get member info
    FILE *file = fopen("members.txt", "r");
    if(file == NULL) {
        perror("Can't open members.txt");
        return 1;
    }

    char **memberInfo = NULL;
    
    memset(row, 0, sizeof(row));
    fgets(row, sizeof(row), file);

    int memberInfo_size = 0;
    while(fscanf(file, "%s", buffer) != EOF) {
        memberInfo = realloc(memberInfo, (memberInfo_size + 1) * sizeof(char*));
        memberInfo[memberInfo_size] = strdup(buffer);
        memset(buffer, 0, BUFFER_SIZE);
    }

    fclose(file);

    // setup socket
    int serverR_socket = set_udp_socket();
    struct sockaddr_in address;
    int addr_len = sizeof(address);

    printf("Server R is up and running using UDP on port %d.\n", serverR_UDP_PORT);

    char client_username[100];

    while(1) {
        memset(&client_username, 0, sizeof(client_username));
        int command_code;
        recvfrom(serverR_socket, command_code, sizeof(command_code), 0, (struct sockaddr*)&address, &addr_len);
        if(command_code == 1) {
            printf("Server R has received a lookup request from the main server.\n");

            recvfrom(serverR_socket, client_username, 100, 0, (struct sockaddr*)&address, (socklen_t*)&addr_len);
            
            int lookup_code = -1;
            for(int i = 0; i < memberInfo_size; i++) {
                if(strcmp(client_username, fileInfo[i].username) == 0) {
                    lookup_code = 0;
                    break;
                }
            }
            if(lookup_code == -1) {
                sendto(serverR_socket, &lookup_code, sizeof(lookup_code), 0, (struct sockaddr*)&address, addr_len);
                continue;
            }

            char **arr = NULL;
            for(int i = 0; i < size; i++) {
                if(strcmp(client_username, fileInfo[i].username) == 0) {
                    arr = realloc(arr, (lookup_code + 1) * sizeof(char*));
                    arr[lookup_code] = strdup(fileInfo[i].filename);
                    n++;
                }
            }

            sendto(serverR_socket, &lookup_code, sizeof(lookup_code), 0, (struct sockaddr*)&address, addr_len);
            for(int i = 0; i < lookup_code; i++) {
                usleep(50000);
                sendto(serverR_socket, arr[i], strlen(arr[i]), 0, (struct sockaddr*)&address, addr_len);
                // printf("%s\n", arr[i]);
                free(arr[i]);
            }

            printf("Server R has finished sending the response to the main server.\n");
        }
    }
    return 0;
}