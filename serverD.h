#include <stdio.h>
#include <stdbool.h>

#define serverD_UDP_PORT 23048
#define BUFFER_SIZE 1024

void file_write(const char *username, const char *filename);
int set_udp_socket();