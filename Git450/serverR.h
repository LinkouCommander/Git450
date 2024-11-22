#include <stdio.h>
#include <stdbool.h>

#define serverR_UDP_PORT 22048
#define BUFFER_SIZE 1024

typedef struct {
    char username[100];
    char filename[100];
} UserFile;

UserFile* read_file_info(UserFile* userfiles, int* size, const char* username, const char *filename);
int set_udp_socket();
char* encoder(char *password);