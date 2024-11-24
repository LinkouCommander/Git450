#include <stdio.h>
#include <stdbool.h>

// #define serverM_TCP_PORT 25048
#define BUFFER_SIZE 1024

void read_command(char *command, char *target);
void lookup_op(int sock, const char *clientname, char *target);
int file_exists(const char *filename);
void push_op(int sock, const char *clientname, const char *target);
void deploy_op(int sock, const char *clientname);
void remove_op(int sock, const char *clientname, const char *target);
void log_op(int sock, const char *clientname);