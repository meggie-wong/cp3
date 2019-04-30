// #include "customsocket.h"
#include <sys/time.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include "logger.h"
#include "dns_helper.h"


#define MAXSERVER 100

typedef struct dns_record {
    char* hostname;
    char* server_ip[MAXSERVER];
    int resolve_cnt;
    int record_cnt;
} dns_record_t;


