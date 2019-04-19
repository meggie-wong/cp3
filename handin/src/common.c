
#include "common.h"
/*
 *
 * 
*/
// char* client_request_handler(client **clients, size_t i, char* request) {
//     char*p = NULL;
//     char * new_request;
//     int bitrate = 0;
//     int seq = 0;
//     int frag = 0;

//     if (!request) {
//         printf("[Error] request should not be NULL");
//         return -1;
//     } 
//     if (p = strstr(request, "GET")) {

//         if (strstr(request, "f4m")) {
            
//         } else if (sscanf(request, "%dSeg%d-Frag%d", &bitrate, &seq, %frag) == 3) {
//             new_request = chunk_request_handler(clients, i, request, bitrate, seq, frag);
//         } else {
//             new_request = request;
//         }
//     }
//     // int bytes_queued = queue_message_send(clients, sibling_idx, msg_rcvd);
//     return new_request;
// }

// void bunny_request_handler(char * request) {
//     // 如果这个请求是server过来的
// }

chunk_node_t* creat_chunk_node(char* request) {
    char*p = NULL;
    int bitrate = 0;
    chunk_node_t* chunk_node = (chunk_node_t*)malloc(sizeof(chunk_node_t));
    chunk_node->next = NULL;
    gettimeofday(&chunk_node->send_time, NULL);
    sscanf(p, "%dSeg%d-Frag%d", &bitrate, &chunk_node->seg, %chunk_node->frag);
    return chunk_node;
}

void push_chunk_into_queue(client **clients, size_t i, chunk_node_t* chunk_node) {
    if (clients[i]->send_queue_tail == NULL) {
        clients[i]->send_queue_head = chunk_node;
        clients[i]->send_queue_tail = chunk_node;
    } else {
        clients[i]->send_queue_tail->next = chunk_node;
        clients[i]->send_queue_tail = clients[i]->send_queue_tail->next;
    }
    return;
}

void pop_chunk_from_queue(client **clients, size_t i) {

    if (clients[i]->send_queue_head == NULL) {
        printf("[Error] Send queue is empty. Pop error!");
        return NULL;
    } else {
        chunk_node_t* head = clients[i]->send_queue_head;
        clients[i]->send_queue_head = clients[i]->send_queue_head->next;
        if (clients[i]->send_queue_head  == NULL) clients[i]->send_queue_tail = NULL;
        free(head);
    }
}

char* chunk_request_handler(client **clients, size_t i, char * request) {
    char* new_request = malloc(1048*sizeof(char));
    struct timeval now, time_elapse;
    chunk_node_t* new_chunk = creat_chunk_node(request);
    push_chunk_into_queue(clients, i, new_chunk);
    sscanf(request, "%dSeg%d-Frag%d", &bitrate, &chunk_node->seg, %chunk_node->frag);

    char *ptr;
    int bitrate = 0;
    int i = 0;
    int cur_throughput = clients[i]->throughput;
    // find the fit bitrate
    for (i = 0; i< clients[i]->number_of_rates; i++) {
        if (clients[i]->bit_rates[i] < cur_throughput/1.5 && clients[i]->bit_rates[i] > bitrate) {
            bitrate = clients[i]->bit_rates[i];
        }
    }
    // replace the bitrate 
    ptr = strrchr(request, '/');
    strncpy(new_request, request, ptr-request);
    sprintf(new_request+ptr-request,"/%dSeg%d-Frag%d", &bitrate, &chunk_node->seg, %chunk_node->frag);

    free(request);
    return new_request;
}








int download_f4m(int server_fd) {
    FILE* fp = fopen(BUDDY_F4M, "w");
    int n;
    char buf[MAXLINE];
    if (fp  == NULL) {
        printf("[Error] error in initializing f4m file");
        return -1;
    }
    char* request = "GET /vod/big_buck_bunny.f4m HTTP/1.1\r\n\r\n";
    send(server_fd, request, strlen(request), 0);

    while ((n = (recv(server_fd, buf, MAXLINE, 0))) > 0) {
        fwrite(buf, 1, n, fp);
        memset(buf, 0, MAXLINE);
    }

    n = fclose(fp);
    if (n != 0) {
        printf("[Error] close f4m file failed");
        return -1;
    }
    return 0;
}


int download_no_list(int server_fd) {
    FILE* fp = fopen(BUDDY_F4M_NOLIST, "w");
    int n;
    char buf[MAXLINE];
    if (fp  == NULL) {
        printf("[Error] error in initializing f4m_nolist file");
        return -1;
    }
    char* request = "GET /vod/big_buck_bunny_nolist.f4m HTTP/1.1\r\n\r\n";
    send(server_fd, request, strlen(request), 0);

    while ((n = (recv(server_fd, buf, MAXLINE, 0))) > 0) {
        fwrite(buf, 1, n, fp);
        memset(buf, 0, MAXLINE);
    }

    n = fclose(fp);
    if (n != 0) {
        printf("[Error] close f4m file failed");
        return -1;
    }
    return 0;
}