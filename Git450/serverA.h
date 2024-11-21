#include <stdio.h>
#include <stdbool.h>

#define serverA_UDP_PORT 21048

typedef struct {
    char UserName[100];
    char Password[100];
} Member;

Member *add_member(Member *members, int *size, const char *UserName, const char *Password);
int set_udp_socket();
char *encoder(char *password);