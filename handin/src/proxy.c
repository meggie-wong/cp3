/********************************************************
*  Author: Sannan Tariq                                *
*  Email: stariq@cs.cmu.edu                            *
*  Description: This code creates a simple             *
*                proxy with some HTTP parsing          *
*                and pipelining capabilities           *
*  Bug Reports: Please send email with                 *
*              subject line "15441-proxy-bug"          *
*  Complaints/Opinions: Please send email with         *
*              subject line "15441-proxy-complaint".   *
*              This makes it easier to set a rule      *
*              to send such mail to trash :)           *
********************************************************/
#include "proxy.h"
#include "logger.h"

char* log_path;
float alpha;
int listen_port;
char* fake_ip;
char* dns_ip;
int dns_port;
char* www_ip;

chunk_node_t* creat_chunk_node(char* request) {
    int bitrate = 0;
    chunk_node_t* chunk_node = (chunk_node_t*)malloc(sizeof(chunk_node_t));
    chunk_node->bitrate = 0;
    chunk_node->next = NULL;
    gettimeofday(&chunk_node->send_time, NULL);
    sscanf(request, "GET /vod/%dSeg%d-Frag%d", &bitrate, &chunk_node->seg, &chunk_node->frag);
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
        return;
    } else {
        chunk_node_t* head = clients[i]->send_queue_head;
        clients[i]->send_queue_head = clients[i]->send_queue_head->next;
        if (clients[i]->send_queue_head  == NULL) clients[i]->send_queue_tail = NULL;
        free(head);
    }
}

char* chunk_request_handler(client **clients, size_t i, char * request) {
    struct timeval now, time_elapse;
    char *ptr;
    int bitrate = 0;
    int idx = 0;
    int cur_throughput = clients[i]->throughput;
    char* new_request = malloc(INIT_BUF_SIZE);
    memset(new_request, '\0', INIT_BUF_SIZE);
    chunk_node_t* chunk_node = creat_chunk_node(request);
    push_chunk_into_queue(clients, i, chunk_node);
    chunk_node_t *tmp = clients[i]->send_queue_head;
    while(tmp) {
        printf("[&]: seg=%d, frag=%d\n", tmp->seg, tmp->frag);
        tmp = tmp->next;
    }
    // find the location of Seq
    char* others = strstr(request, "Seg");
    if (others == NULL) {
        printf("[Error] Can not parse out 'Seg'!\n");
        return request;
    }
    sscanf(others, "Seg%d-Frag%d", &chunk_node->seg, &chunk_node->frag);
    ptr = others;

    while(ptr > request) {
        if(*ptr == '/') break;
        ptr --;
    }
    if(sscanf(ptr, "/%d", &bitrate) != 1) {
        printf("[Error] Can not parse out original bitrate!\n");
        return request;
    }
    *ptr = '\0';
    strcpy(new_request, request);

    printf("[*]parse: %s, %lu, %d, %d, %d\n", new_request, strlen(new_request), bitrate, chunk_node->seg, chunk_node->frag);
    printf("clients[i]->number_of_rates = %d\n", clients[i]->number_of_rates);
    // find the fit bitrate
    if (cur_throughput == 0) {
        bitrate = 10000000;
    } else {
        bitrate = 0;
    }
    for (idx = 0; idx< clients[i]->number_of_rates; idx++) {
        printf("clients[i]->bit_rates[idx] = %d T_cur = %d\n", clients[i]->bit_rates[idx],cur_throughput);
        if (cur_throughput == 0) {
            // pick the minimal birtate
            if (clients[i]->bit_rates[idx] < bitrate) 
                bitrate = clients[i]->bit_rates[idx];
        }
        if (clients[i]->bit_rates[idx] < cur_throughput/1.5 && clients[i]->bit_rates[idx] > bitrate) {
            bitrate = clients[i]->bit_rates[idx];
        }
    }
    if(bitrate == 0) {
        for (idx = 0; idx< clients[i]->number_of_rates; idx++) {
            // pick the minimal birtate
            if (clients[i]->bit_rates[idx] < bitrate) 
                bitrate = clients[i]->bit_rates[idx];
        }
    }
    chunk_node->bitrate = bitrate;
    
    char buf[INIT_BUF_SIZE];
    memset(buf, '\0', INIT_BUF_SIZE);
    sprintf(buf, "/%d", bitrate);
    printf("%s\n%s\n", buf, new_request);
    strcat(new_request, buf);
    strcat(new_request, others);
    // sprintf(new_request+strlen(new_request),"%dSeg%d-Frag%d", bitrate, chunk_node->seg, chunk_node->frag);

    free(request);
    return new_request;
}

/*
 *  @REQUIRES:
 *  client_fd: The fd of the client you want to add
 *  is_server: A flag to tell us whether this client is a server or a requester
 *  sibling_idx: For a server it will be its client, for a client it will be its server
 *  
 *  @ENSURES: returns a pointer to a new client struct
 *
*/
client *new_client(int client_fd, int is_server, size_t sibling_idx) {
    client *new = calloc(1, sizeof(client));
    new->fd = client_fd;
    new->recv_buf = calloc(INIT_BUF_SIZE, 1);
    new->send_buf = calloc(INIT_BUF_SIZE, 1);
    new->recv_buf_len = 0;
    new->send_buf_len = 0;
    new->recv_buf_size = INIT_BUF_SIZE;
    new->send_buf_size = INIT_BUF_SIZE;
    new->sibling_idx = sibling_idx;
    new->send_queue_head = NULL;
    new->send_queue_tail = NULL;
    new->throughput = 0;
    new->number_of_rates = 4;
    new->bit_rates[0] = 10;
    new->bit_rates[1] = 100;
    new->bit_rates[2] = 500;
    new->bit_rates[3] = 1000;
    return new;
}

void free_client(client* c) {
    free(c->recv_buf);
    free(c->send_buf);
    free(c);
    return;
}

/*
 *  @REQUIRES:
 *  client_fd: The fd of the client you want to add
 *  clients: A pointer to the array of client structures
 *  read_set: The set we monitor for incoming data
 *  is_server: A flag to tell us whether this client is a server or a requester
 *  sibling_idx: For a server it will be its client's index, for a client it will be its server index
 *  
 *  @ENSURES: Returns the index of the added client if possible, otherwise -1
 *
*/
int add_client(int client_fd, client **clients, fd_set *read_set, int is_server, size_t sibling_idx) {
    int i;
    for (i = 0; i < MAX_CLIENTS - 1; i ++) {
        if (clients[i] == NULL) {
            clients[i] = new_client(client_fd, is_server, sibling_idx);
            FD_SET(client_fd, read_set);
            return i;
        }
    }
    return -1;
}

/*
 *  @REQUIRES:
 *  clients: A pointer to the array of client structures
 *  i: Index of the client to remove
 *  read_set: The set we monitor for incoming data
 *  write_set: The set we monitor for outgoing data
 *  
 *  @ENSURES: Removes the client and its sibling from all our data structures
 *
*/
int remove_client(client **clients, size_t i, fd_set *read_set, fd_set *write_set) {
    
    if (clients[i] == NULL) {
        return -1;
    }
    printf("Removing client on fd: %d\n", clients[i]->fd);
    int sib_idx;
    close(clients[i]->fd);
    FD_CLR(clients[i]->fd, read_set);
    FD_CLR(clients[i]->fd, write_set);
    sib_idx = clients[i]->sibling_idx;
    if (clients[sib_idx] != NULL) {
        printf("Removing client on fd: %d\n", clients[sib_idx]->fd);
        close(clients[sib_idx]->fd);
        FD_CLR(clients[sib_idx]->fd, read_set);
        FD_CLR(clients[sib_idx]->fd, write_set);
        free_client(clients[sib_idx]);
        clients[sib_idx] = NULL;
    }
    free_client(clients[i]);
    clients[i] = NULL;
    return 0;
}

int find_maxfd(int listen_fd, client **clients) {
    int max_fd = listen_fd;
    int i;
    for (i = 0; i < MAX_CLIENTS - 1; i ++) {
        if (clients[i] != NULL) {
            if (max_fd < clients[i]->fd) {
                max_fd = clients[i]->fd;
            }
        }
    }
    return max_fd;
}


/*
 *  @REQUIRES:
 *  clients: A pointer to the array of client structures
 *  i: Index of the client to remove
 *  
 *  @ENSURES: 
 *  - tries to send the data present in a clients send buffer to that client
 *  - If data is sent, returns the remaining bytes to send, otherwise -1
 *
*/
int process_client_send(client **clients, size_t i) {
    int n;
    char *new_send_buffer;
    
    n = send(clients[i]->fd, clients[i]->send_buf, clients[i]->send_buf_len, 0);

    if (n <= 0) {
        return -1;
    }

    size_t new_size = max(INIT_BUF_SIZE, clients[i]->send_buf_len - n);
    new_send_buffer = calloc(new_size, sizeof(char));
    memcpy(new_send_buffer, clients[i]->send_buf + n, clients[i]->send_buf_len - n);
    free(clients[i]->send_buf);
    clients[i]->send_buf = new_send_buffer;
    clients[i]->send_buf_len = clients[i]->send_buf_len - n;
    clients[i]->send_buf_size = new_size;

    return clients[i]->send_buf_len;
}

/*
 *  @REQUIRES:
 *  clients: A pointer to the array of client structures
 *  i: Index of the client to remove
 *  
 *  @ENSURES: 
 *  - tries to recv data from the client and updates its internal state as appropriate
 *  - If data is received, return the number of bytes received, otherwise return 0 or -1
 *
*/
int recv_from_client(client** clients, size_t i) {
    int n;
    char buf[INIT_BUF_SIZE];
    size_t new_size;

    n = recv(clients[i]->fd, buf, INIT_BUF_SIZE, 0);

    
    if (n <= 0) {
        return n;
    }

    new_size = clients[i]->recv_buf_size;

    while (n > new_size - clients[i]->recv_buf_len) {
        new_size *= 2;
        
    }
    clients[i]->recv_buf = resize(clients[i]->recv_buf, 
        new_size, clients[i]->recv_buf_size);
    clients[i]->recv_buf_size = new_size;

    memcpy(&(clients[i]->recv_buf[clients[i]->recv_buf_len]), buf, n);
    clients[i]->recv_buf_len += n;

    return n;
}


/*
 *  @REQUIRES:
 *  clients: A pointer to the array of client structures
 *  i: Index of the client to remove
 *  buf: The message to add to the send buffer
 *  
 *  @ENSURES: 
 *  - appends data to the client's send buffer and returns the number of bytes appended
 *
*/
int queue_message_send(client **clients, size_t i, pop_response res) {
    size_t n = res.message_length;
    size_t new_size;

    new_size = clients[i]->send_buf_size;

    while (n > new_size - clients[i]->send_buf_len) {
        new_size *= 2;
        
    }
    clients[i]->send_buf = resize(clients[i]->send_buf, 
        new_size, clients[i]->send_buf_size);
    clients[i]->send_buf_size = new_size;

    memcpy(&(clients[i]->send_buf[clients[i]->send_buf_len]), res.message, n);
    clients[i]->send_buf_len += n;
    return n;
}

void server_response_handler(client **clients, size_t i, pop_response res) {
    char* response = res.message;
    char* content_type = get_content_type(response, res.message_length);
    if(content_type!= NULL && strstr(content_type, "video/f4f")) {
        float time_out;
        int new_throughput;
        struct timeval now, time_elapse;
        chunk_node_t*head_chunk = clients[clients[i]->sibling_idx]->send_queue_head;
        if (head_chunk == NULL) {
            printf("[Error] The head of the send queue is empty. Cannot update the throughput!");
            return;
        }
        gettimeofday(&now, NULL);
        timersub(&now, &head_chunk->send_time, &time_elapse);
        time_out = (time_elapse.tv_sec + (1.0 * time_elapse.tv_usec)/1000000);
        printf("time out is %2f\n", time_out);
        new_throughput = 8*get_content_length(response, res.message_length)/(time_out*1000);
        clients[clients[i]->sibling_idx]->throughput = alpha*new_throughput + (1-alpha)*clients[clients[i]->sibling_idx]->throughput;
        
        printf("Updated throughput is %d\n", clients[clients[i]->sibling_idx]->throughput);
        logger("%ld %f %d %d %d %s %dSeg%d-Frag%d\n",
            (long)now.tv_sec, time_out, new_throughput, clients[clients[i]->sibling_idx]->throughput, 
            head_chunk->bitrate, www_ip, head_chunk->bitrate, head_chunk->seg, head_chunk->frag);
        pop_chunk_from_queue(clients, clients[i]->sibling_idx);
    }
}

/*
 *  @REQUIRES:
 *  clients: A pointer to the array of client structures
 *  i: Index of the client to remove
 *  data_available: flag whether you can call recv on this client without blocking
 *  write_set: the set containing the fds to observe for being ready to write to
 *  
 *  @ENSURES: 
 *  - tries to read data from the client, then tries to reap a complete http message
 *      and finally tries to queue the message to be forwarded to its sibling
 *  - returns number of bytes queued if no errors, -1 otherwise
 *
*/
int process_client_read(client **clients, size_t i, int data_available, fd_set *write_set) {
    char *msg_rcvd;
    int nread;
    char new_request[INIT_BUF_SIZE];
    int bytes_queued = 0;
    char* p = NULL;
    int rate;
    int sibling_idx = clients[i]->sibling_idx;
    if (data_available == 1) {
        if ((nread = recv_from_client(clients, i)) < 0) {
            fprintf(stderr, "start_proxying: Error while receiving from client\n");
            return -1;
        }
        else if (nread == 0) {
            return -1;
        }
    }

    pop_response res = pop_message(&(clients[i]->recv_buf), &(clients[i]->recv_buf_len), &clients[i]->recv_buf_size);

    if ((msg_rcvd = res.message) == NULL) {
        return 0;
    }

    else {
        if (clients[i]->is_server) {
            // 如果这个请求是server过来的
            if (strstr(msg_rcvd, "bitrate=\"") != NULL) {
                printf("[Message] Receive f4m file from server, start to proces it\n");
                // 是真实的f4m文件，不转发回去
                size_t nlen = strlen("bitrate:");
                p = msg_rcvd;
                int count = 0;
                p = strstr(p, "bitrate=\"");
                while (p != NULL) {
                    sscanf(p, "bitrate=\"%d", &rate);
                    clients[sibling_idx]->bit_rates[count++] = rate;
                    p += nlen;
                    p = strstr(p, "bitrate=\"");
                }
                clients[sibling_idx]->number_of_rates = count;
                int dummy;
                for (dummy = 0; dummy < count; dummy++) {
                    // printf("Bit Rate %d is %d\n", dummy, clients[clients[i]->sibling_idx]->bit_rates[dummy]);
                    printf("Bit Rate %d is %d\n", dummy, clients[sibling_idx]->bit_rates[dummy]);
                }

                return 0;
            } else {
                // 其他的文件都正常的转发回去
                printf("[Message] Forward other response from server to client\n");
                server_response_handler(clients, i, res);
            }
        } else {
            // 如果这个请求是client过来的
            if (strstr(msg_rcvd, ".f4m") != NULL && strstr(msg_rcvd, "GET") != NULL) {
                printf("[Message] client request for f4m file\n");
                // 这个请求是GET f4m 文件， 多queue一个nolist的request
                pop_response tmp;
                char new_message[INIT_BUF_SIZE];
                memset(new_message, 0, INIT_BUF_SIZE);
                p = strstr(msg_rcvd, ".f4m");
                memcpy(new_message, msg_rcvd, p - msg_rcvd);
                strcat(new_message, "_nolist");
                strcat(new_message, p);
                tmp.message = new_message;
                tmp.message_length = strlen(tmp.message);
                bytes_queued += queue_message_send(clients, sibling_idx, tmp);
                printf("Content is:\n%s\n", tmp.message);
            } else {
                // 其他request都直接转发到server
                // TODO: 需要处理一下bitrate的重新计算问题
                if (strstr(msg_rcvd, "GET") != NULL && 
                    strstr(strstr(msg_rcvd, "GET"), "Seg") != NULL && 
                    strstr(strstr(strstr(msg_rcvd, "GET"), "Seg"), "-Frag") != NULL) {

                    printf("[Message] client has chunk request, modify it and send to server\n");
                    res.message = chunk_request_handler(clients, i, msg_rcvd);
                    res.message_length = strlen(res.message);
                } else {
                    printf("[Message] client has other request, forward to server\n");
                }
            }
        }
        printf("[Message] %s\n", res.message);
        bytes_queued += queue_message_send(clients, sibling_idx, res);
        FD_SET(clients[sibling_idx]->fd, write_set);
        free(res.message);
        return bytes_queued;
    }

}

int start_proxying() {
    int max_fd, nready, listen_fd;
    fd_set read_set, read_ready_set, write_set, write_ready_set;
    struct sockaddr_in cli_addr;
    socklen_t cli_size;
    client **clients;
    size_t i;

    // listen_port = 8888;
    char* server_ip = "127.0.0.1";
    if (www_ip != NULL) {
        server_ip = www_ip;
    }
    unsigned short server_port = 8080;
    char *my_ip = fake_ip;

    if ((listen_fd = open_listen_socket(listen_port)) < 0) {
        fprintf(stderr, "start_proxy: Failed to start listening\n");
        return -1;
    }


    // init_multiplexer(listen_fd, clients, read_set, write_set);

    clients = calloc(MAX_CLIENTS - 1, sizeof(client*));
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_SET(listen_fd, &read_set);
    
    max_fd = listen_fd;
    printf("Initiating select loop\n");
    while (1) {
        read_ready_set = read_set;
        write_ready_set = write_set;
        // printf("Watining to select...\n");
        nready = select(max_fd+1, &read_ready_set, &write_ready_set, NULL, NULL);

        if (nready > 0) {
            if (FD_ISSET(listen_fd, &read_ready_set)) {
                int client_fd;
                int client_idx;
                nready --;

                cli_size = sizeof(cli_addr);

                if ((client_fd = accept(listen_fd, (struct sockaddr *) &cli_addr,
                                    &cli_size)) == -1) {
                    fprintf(stderr, "start_proxying: Failed to accept new connection");
                }

                // add the client to the client_fd list of filed descriptors
                else if ((client_idx = add_client(client_fd, clients, &read_set, 0, -1))!= -1) {
                    char response_ip[MAXLINE];
                    memset(response_ip, 0, MAXLINE);
                    resolve("video.cmu.cs.edu", response_ip);
                    int sibling_fd = open_socket_to_server(my_ip, server_ip, server_port);
                    int server_idx = add_client(sibling_fd, clients, &read_set, 1, client_idx);
                    clients[client_idx]->sibling_idx = server_idx;
                    printf("start_proxying: Connected to %s on FD %d\n"
                    "And its sibling %s on FD %d\n", inet_ntoa(cli_addr.sin_addr),
                        client_fd, server_ip, sibling_fd);
                    clients[client_idx]->is_server = 0;
                    clients[server_idx]->is_server = 1;

                }
                else
                    close(client_fd);         
            }

            for (i = 0; i < MAX_CLIENTS - 1 && nready > 0; i++) {
                if (clients[i] != NULL) {
                    int data_available = 0;
                    if (FD_ISSET(clients[i]->fd, &read_ready_set)) {
                        nready --;
                        data_available = 1;
                    }

                    int nread = process_client_read(clients, i, data_available, &write_set);

                    if (nread < 0) {
                        if (remove_client(clients, i, &read_set, &write_set) < 0) {
                            fprintf(stderr, "start_proxying: Error removing client\n");
                        }
                    }

                    if (nread >= 0 && nready > 0) {
                        if (FD_ISSET(clients[i]->fd, &write_ready_set)) {
                            nready --;
                            int nsend = process_client_send(clients, i);
                            if (nsend < 0) {
                                if (remove_client(clients, i, &read_set, &write_set) < 0) {
                                    fprintf(stderr, "start_proxying: Error removing client\n");
                                }
                            }                    
                            else if (nsend == 0) {
                                FD_CLR(clients[i]->fd, &write_set);
                            }
                        }
                    }
                }
                
            }
            max_fd = find_maxfd(listen_fd, clients);
        }

    }
}



int main(int argc, char* argv[]) {
    // start_proxying();
    printf("Starting the proxy...\n");

    if (argc < 7) {
        printf("[Usage] ./proxy <log> <alpha> <listen-port> <fake-ip> <dns-ip> <dns-port> [<www-ip>]\n");
        printf("provide %d args now\n", argc);
        return -1;
    }
    log_path = argv[1];
    alpha = atof(argv[2]);
    listen_port = atoi(argv[3]);
    fake_ip = argv[4];
    dns_ip = argv[5];
    dns_port = atoi(argv[6]);
    www_ip = argv[7];
    init_log(log_path);
    init_mydns(dns_ip, dns_port, fake_ip);
    // printf("finish parse the args\n");
    // ======================================== test_only ======================================== 
    char response_ip[MAXLINE];
    memset(response_ip, 0, MAXLINE);
    resolve("video.cmu.cs.edu", response_ip);

    memset(response_ip, 0, MAXLINE);
    resolve("www.cmu.cs.edu", response_ip);
    // ======================================== end test ======================================== 

    // start_proxying();
    return 0;
}