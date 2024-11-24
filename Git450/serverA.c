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
#include "serverA.h"

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
    address.sin_port = htons(serverA_UDP_PORT);
    
    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Socket Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

char* encoder(char *password) {
    int length = strlen(password);
    char *str = (char*)malloc((length + 1) * sizeof(char));
    for(int i = 0; i < length; i++) {
        char c = password[i];

        if(c >= 'A' && c <= 'Z') {
            c = ((c - 'A' + 3) % 26) + 'A';
        }
        else if(c >= 'a' && c <= 'z') {
            c = ((c - 'a' + 3) % 26) + 'a';
        }
        else if(c >= '0' && c <= '9') {
            c = ((c - '0' + 3) % 10) + '0';
        }
        // printf("%c\n", c);
        str[i] = c;
    }
    str[length] = '\0';
    return str;
}

int main() {
    char buffer1[BUFFER_SIZE] = {0};
    char buffer2[BUFFER_SIZE] = {0};

    // get member info
    FILE *file = fopen("members.txt", "r");
    if(file == NULL) {
        perror("Can't open members.txt");
        return 1;
    }

    char **memberInfo = NULL;
    char **passwordInfo = NULL;
    
    char header[100];
    fgets(header, sizeof(header), file);
    header[strcspn(header, "\n")] = 0;

    int member_size = 0;
    while(fscanf(file, "%s %s", buffer1, buffer2) != EOF) {
        memberInfo = realloc(memberInfo, (member_size + 1) * sizeof(char*));
        memberInfo[member_size] = strdup(buffer1);
        memset(buffer1, 0, BUFFER_SIZE);

        passwordInfo = realloc(passwordInfo, (member_size + 1) * sizeof(char*));
        passwordInfo[member_size] = strdup(buffer2);
        memset(buffer2, 0, BUFFER_SIZE);
        member_size++;
    }
    fclose(file);

    // setup socket
    int serverA_socket = set_udp_socket();
    struct sockaddr_in address;
    int addr_len = sizeof(address);

    printf("Server A is up and running using UDP on port %d.\n", serverA_UDP_PORT);

    char client_username[100];
    char client_password[100];

    while(1) {
        memset(&client_username, 0, sizeof(client_username));
        memset(&client_password, 0, sizeof(client_password));
        
        recvfrom(serverA_socket, client_username, 100, 0, (struct sockaddr*)&address, (socklen_t*)&addr_len);
        recvfrom(serverA_socket, client_password, 100, 0, (struct sockaddr*)&address, (socklen_t*)&addr_len);
        printf("Server A received username %s and password ******\n", client_username);

        int authenticationCode = 0;

        int idx = -1;
        for(int i = 0; i < member_size; i++) {
            if(strcmp(client_username, memberInfo[i]) == 0) {
                idx = i;
                break;
            }
        }

        if(idx != -1) {
            char *encoded_client_password = encoder(client_password);
            // printf("Decoded Password: %s\n", decoded_client_password);
            if(strcmp(encoded_client_password, passwordInfo[idx]) == 0) {
                authenticationCode = 2;
            }
        }
        // printf("Authentication Code: %d\n", authenticationCode);
        if(authenticationCode) printf("Member %s has been authenticated\n", client_username);
        else printf("The username %s or password ****** is incorrect\n", client_username);
            
        sendto(serverA_socket, &authenticationCode, sizeof(authenticationCode), 0, (struct sockaddr*)&address, addr_len);
    }

    free(memberInfo);
    free(passwordInfo);
    
    close(serverA_socket);
    return 0;
}