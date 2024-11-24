#include <stdio.h>
#include <stdbool.h>

#define serverA_UDP_PORT 21048
#define BUFFER_SIZE 1024

int set_udp_socket();
char *encoder(char *password);