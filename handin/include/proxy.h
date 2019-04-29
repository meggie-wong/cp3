// #include "common.h"
#include "httpparser.h"
#include "customsocket.h"
#include "logger.h"
#include <sys/time.h>
#include "mydns.h"

#define INIT_BUF_SIZE 8192
#define MAX_CLIENTS FD_SETSIZE
#define MAX_BIT_RATES_TYPES 128

struct chunk_node
{
    struct timeval send_time;
    int seg;
    int frag;
    int bitrate;
    struct chunk_node*next;
};

struct client_struct
{
	int fd;
	char *recv_buf;
    char *send_buf;
    size_t recv_buf_len;
    size_t recv_buf_size;
    size_t send_buf_len;
    size_t send_buf_size;
    size_t is_server;
    size_t sibling_idx;
    int bit_rates[MAX_BIT_RATES_TYPES];
    int number_of_rates;
    int throughput;
    struct chunk_node* send_queue_head;
    struct chunk_node* send_queue_tail;
};

typedef struct client_struct client;
typedef struct chunk_node chunk_node_t;

int start_proxying();