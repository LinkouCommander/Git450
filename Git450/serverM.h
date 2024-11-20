#include <stdio.h>
#include <stdlib.h>

const int serverA_UDP_PORT = 21048;
const int serverR_UDP_PORT = 22048;
const int serverD_UDP_PORT = 23048;
const int serverM_UDP_PORT = 24048;
const int serverM_TCP_PORT = 25048;

struct sockaddr_in udp_client_address;
int udp_client_len = sizeof(udp_client_address);
int set_udp_socket();
int set_tcp_socket();
int set_tcp_client_socket(int tcp_server_socket);
