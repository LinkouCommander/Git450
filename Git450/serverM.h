#include <stdio.h>
#include <stdlib.h>

#define serverA_UDP_PORT 21048
#define serverR_UDP_PORT 22048
#define serverD_UDP_PORT 23048
#define serverM_UDP_PORT 24048
#define serverM_TCP_PORT 25048

#define BUFFER_SIZE 1024
#define LOG_SIZE 1024

typedef struct {
    char username[100];
    char command[100];
    char target[100];
} Command;

Command* add_command(Command* logs, int* size, const char* username, const char* command, const char* target);
struct sockaddr_in udp_client_address[3];
int udp_client_len = sizeof(udp_client_address[0]);
int set_udp_socket();
int set_tcp_socket();
int set_tcp_client_socket(int tcp_server_socket);