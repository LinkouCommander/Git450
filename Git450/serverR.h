#ifndef SERVER_R_H
#define SERVER_R_H

#include <stdio.h>
#include <stdbool.h>

#define serverR_UDP_PORT 22048
#define BUFFER_SIZE 1024

void file_write(const char *username, const char *filename);
int set_udp_socket();

#endif