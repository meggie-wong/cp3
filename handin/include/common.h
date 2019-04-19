// #include "log.h"
#include "proxy.h"
#include <stdio.h>


#define BUDDY_F4M "big_buck_bunny.f4m"
#define BUDDY_F4M_NOLIST "big_buck_bunny_nolist.f4m"

char* client_request_handler(client **clients, size_t i, char* request);
void server_response_handler(client **clients, size_t i, char* response);
chunk_node_t* creat_chunk_node(char* request);
void push_chunk_into_queue(client **clients, size_t i, chunk_node_t* chunk_node);
void pop_chunk_from_queue(client **clients, size_t i);
char* chunk_request_handler(client **clients, size_t i, char * request);

int download_f4m(int server_fd);
int process_f4m(client* server);
int download_no_list(int server_fd);
void print_rates(client* server);