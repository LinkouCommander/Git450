#include <stdio.h>
#include <stdlib.h>

#define serverA_UDP_PORT 21048
#define serverR_UDP_PORT 22048
#define serverD_UDP_PORT 23048
#define serverM_UDP_PORT 24048
#define serverM_TCP_PORT 25048

#define BUFFER_SIZE 1024

struct sockaddr_in udp_client_address[3];
int udp_client_len = sizeof(udp_client_address[0]);
int set_udp_socket();
int set_tcp_socket();
int set_tcp_client_socket(int tcp_server_socket);
