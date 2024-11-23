#include <stdio.h>
#include <stdbool.h>

#define serverM_TCP_PORT 25048
#define BUFFER_SIZE 1024

void read_command(char *command, char *target);
void lookup_op(int sock, const char *clientname, char *target);