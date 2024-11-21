#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "serverR.h"

#define TABLE_SIZE 2001

typedef struct {
    char *key;
    char **value;
    size_t node_size;
    struct Node *next;
} Node;

typedef struct {
    Node **table;
    size_t size;
} UnorderedMap;

size_t hash_string(const char *key) {
    int hash = 0;
    while(*key) {
        hash = (hash * 31) + *key;
        *key++;
    }
    return hash % TABLE_SIZE;
}

UnorderedMap *create_map() {
    UnorderedMap* map = (UnorderedMap*)malloc(sizeof(UnorderedMap));
    map->size = size;
    map->table = (Node**)malloc(size * sizeof(Node*));

    for(size_t i = 0; i < TABLE_SIZE; i++) {
        map->table[i] = NULL;
    }
    return map;
}

void insert(UnorderedMap *map, const char *key, const char *value) {
    size_t idx = hash_string(key);
    Node *node = map->table[idx];
    Node *prev = NULL;

    while(node) {
        if(strcmp(node->key, key) == 0) {
            for(size_t i = 0; i < node->node_size; i++) {
                if(strcmp(node->value, value) == 0) {
                    return;
                }
            }
            node->value = realloc(node->value, (node->node_size + 1) * sizeof(node->value));
            // strcpy(node->value[node->node_size], value);
            node->value[node->node_size] = strdup(value);
            node->node_size++;
            return;
        }
        prev = node;
        node = node->next;
    }

    Node *new_node = (Node*)malloc(sizeof(Node));
    new_entry->key.str = strdup(key);
    new_node->value = (char**)malloc(sizeof(char*));
    new_node->value[0] = strdup(value);
    new_node->node_size = 1;
    new_node->next = NULL;

    if(prev) prev->next = new_node;
    else map->table[idx] = new_node;
}

void print_hashmap(HashMap* hashmap) {
    for (size_t i = 0; i < hashmap->size; i++) {
        HashEntry* entry = hashmap->table[i];
        while (entry) {
            printf("Key: %s\n", entry->key.str);
            printf("Values: ");
            for (size_t j = 0; j < entry->value_count; j++) {
                printf("%s ", entry->values[j].str);
            }
            printf("\n");
            entry = entry->next;
        }
    }
}

void free_hashmap(HashMap* hashmap) {
    for (size_t i = 0; i < hashmap->size; i++) {
        HashEntry* entry = hashmap->table[i];
        while (entry) {
            free(entry->key.str);
            for (size_t j = 0; j < entry->value_count; j++) {
                free(entry->values[j].str);
            }
            free(entry->values);
            HashEntry* temp = entry;
            entry = entry->next;
            free(temp);
        }
    }
    free(hashmap->table);
    free(hashmap);
}

int set_udp_socket() {
    int sockfd;
    struct sockaddr_in address;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(serverR_UDP_PORT);
    
    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Socket Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int main() {
    FILE *file = fopen("filenames.txt", "r");
    if(file == NULL) {
        perror("Can't open members.txt");
        return 1;
    }

    char username[100];
    char filename[100];
    
    char row[1];
    fgets(row, sizeof(row), file);

    int size = 0;
    UnorderedMap* mp = create_map();
    while(fscanf(file, "%s %s", username, filename) != EOF) {
        insert(mp, username, filename);
    }

    fclose(file);

    int serverR_socket = set_udp_socket();
    struct sockaddr_in address;
    int addr_len = sizeof(address);

    printf("Server R is up and running using UDP on port %d.\n", serverR_UDP_PORT);

    char client_username[100];

    while(1) {
        memset(&client_username, 0, sizeof(client_username));
        recvfrom(serverR_socket, client_username, 100, 0, (struct sockaddr*)&address, (socklen_t*)&addr_len);
        print_hashmap(mp);
    }
    free_hashmap(mp);
    return 0;
}