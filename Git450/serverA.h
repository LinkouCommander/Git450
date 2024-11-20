#include <stdio.h>
#include <stdbool.h>

const int serverA_UDP_PORT = 21048;

struct Member {
    char UserName[100];
    char Password[100];
};

struct Member* add_member(struct Member* members, int* size, const char* UserName, const char *Password);
int set_udp_socket();
char* encoder(char *password);