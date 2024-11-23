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

void file_write(const char *username, const char *filename) {
    FILE *push_file = fopen("filenames.txt", "a");
    if(!push_file) {
        perror("Can't open filenames.txt");
        return 1;
    }
    // fprintf(push_file, "%s %s\n", username, filename);
    printf("%s %s\n", username, filename);
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
    char buffer1[BUFFER_SIZE] = {0};
    char buffer2[BUFFER_SIZE] = {0};

    // get member info
    FILE *memberfile = fopen("members.txt", "r");
    if(memberfile == NULL) {
        perror("Can't open members.txt");
        return 1;
    }

    char **memberInfo = NULL;
    
    memset(row, 0, sizeof(row));
    fgets(row, sizeof(row), memberfile);

    int member_size = 0;
    while(fscanf(memberfile, "%s %s", buffer1, buffer2) != EOF) {
        memberInfo = realloc(memberInfo, (member_size + 1) * sizeof(char*));
        memberInfo[member_size] = strdup(buffer1);
        memset(buffer1, 0, BUFFER_SIZE);
        memset(buffer2, 0, BUFFER_SIZE);
        member_size++;
    }
    fclose(memberfile);

    // setup socket
    int serverR_socket = set_udp_socket();
    struct sockaddr_in address;
    int addr_len = sizeof(address);

    printf("Server R is up and running using UDP on port %d.\n", serverR_UDP_PORT);


    while(1) {
        // get file info
        FILE *file = fopen("filenames.txt", "r");
        if(!file) {
            perror("Can't open filenames.txt");
            return 1;
        }

        char **fileUser = NULL;
        char **fileInfo = NULL;
        
        char row[1];
        fgets(row, sizeof(row), file);

        int file_size = 0;
        while(fscanf(file, "%s %s", buffer1, buffer2) != EOF) {
            fileUser = realloc(fileUser, (file_size + 1) * sizeof(char*));
            fileUser[file_size] = strdup(buffer1);
            memset(buffer1, 0, BUFFER_SIZE);

            fileInfo = realloc(fileInfo, (file_size + 1) * sizeof(char*));
            fileInfo[file_size] = strdup(buffer2);
            memset(buffer2, 0, BUFFER_SIZE);
            file_size++;
        }
        fclose(file);

        // receive command
        int command_code;
        recvfrom(serverR_socket, &command_code, sizeof(command_code), 0, (struct sockaddr*)&address, &addr_len);
        if(command_code == 1) {
            printf("Server R has received a lookup request from the main server.\n");

            char client_username[100];
            memset(&client_username, 0, sizeof(client_username));
            recvfrom(serverR_socket, client_username, sizeof(client_username), 0, (struct sockaddr*)&address, &addr_len);
            
            int lookup_code = -1;
            for(int i = 0; i < member_size; i++) {
                if(strcmp(client_username, memberInfo[i]) == 0) {
                    lookup_code = 0;
                    break;
                }
            }
            if(lookup_code == -1) {
                sendto(serverR_socket, &lookup_code, sizeof(lookup_code), 0, (struct sockaddr*)&address, addr_len);
                continue;
            }

            char **arr = NULL;
            for(int i = 0; i < file_size; i++) {
                if(strcmp(client_username, fileUser[i]) == 0) {
                    arr = realloc(arr, (lookup_code + 1) * sizeof(char*));
                    arr[lookup_code] = strdup(fileInfo[i]);
                    // printf("row %d: %s\n", i, arr[lookup_code]);
                    lookup_code++;
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
        else if(command_code == 2) {
            printf("Server R has received a push request from the main server");

            char client_username[100];
            memset(&client_username, 0, sizeof(client_username));
            recvfrom(serverR_socket, client_username, sizeof(client_username), 0, (struct sockaddr*)&address, &addr_len);
            
            char client_filename[100];
            memset(&client_filename, 0, sizeof(client_filename));
            recvfrom(serverR_socket, client_filename, sizeof(client_filename), 0, (struct sockaddr*)&address, &addr_len);
            
            int response_code = 0;
            for(int i = 0; i < file_size; i++) {
                if(strcmp(client_username, fileUser[i]) == 0 && strcmp(client_filename, fileInfo[i]) == 0) {
                    response_code = 1;
                    break;
                }
            }

            sendto(serverR_socket, &response_code, sizeof(response_code), 0, (struct sockaddr*)&address, addr_len);
            if(!response_code) {
                file_write(client_username, client_filename);
                printf("%s uploaded successfully\n", client_filename);
            }
            else {
                printf("%s exists in %s's repository; requesting overwrite confirmation.\n", client_filename, client_username);

                int overwrite_code;
                recvfrom(serverR_socket, &overwrite_code, sizeof(overwrite_code), 0, (struct sockaddr*)&address, &addr_len);

                if(overwrite_code == 1) {
                    printf("User requested overwrite; overwrite successful.\n");
                }
                else if(overwrite_code == 0) {
                    printf("Overwrite denied\n");
                }
            }
        }
        free(fileUser);
        free(fileInfo);
    }

    free(memberInfo);

    close(serverR_socket);

    return 0;
}