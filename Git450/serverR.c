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
    FILE *file = fopen("filenames.txt", "r");
    if(file == NULL) {
        perror("Can't open members.txt");
        return 1;
    }

    char username[100];
    char filename[100];
    
    char row[1];
    fgets(row, sizeof(row), file);

    int size = 0;
    UserFile* fileinfo = NULL;
    while(fscanf(file, "%s %s", username, filename) != EOF) {
        fileinfo = read_file(fileinfo, &size, username, filename);
    }

    fclose(file);

    int serverR_socket = set_udp_socket();
    struct sockaddr_in address;
    int addr_len = sizeof(address);

    printf("Server R is up and running using UDP on port %d.\n", serverR_UDP_PORT);

    char client_username[100];

    while(1) {
        memset(&client_username, 0, sizeof(client_username));
        recvfrom(serverR_socket, client_username, 100, 0, (struct sockaddr*)&address, (socklen_t*)&addr_len);
        
        int n = 0;
        char **arr = NULL;
        for(int i = 0; i < size; i++) {
            if(strcmp(client_username, fileinfo[i].username) == 0) {
                realloc(arr, (n + 1) * sizeof(char*));
                arr[n] = strdup(fileinfo[i].filename);
                n++;
            }
        }

        sendto(serverR_socket, &n, sizeof(n), 0, (struct sockaddr*)&address, addr_len);
        for(int i = 0; i < n; i++) {
            usleep(50000);
            sendto(serverR_socket, arr[i], sizeof(arr[i]), 0, (struct sockaddr*)&address, addr_len);
            free(arr[i]);
        }
    }
    return 0;
}