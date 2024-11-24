#include <stdio.h>
#include <stdbool.h>

#define serverR_UDP_PORT 22048
#define BUFFER_SIZE 1024

void file_write(const char *username, const char *filename);
void delete_line(char **fileUser, char **fileInfo, const int size, const char* row, int line_to_delete);
int set_udp_socket();