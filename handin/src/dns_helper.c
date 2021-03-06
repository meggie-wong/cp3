// #include "dns_helper.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dns_helper.h"


/**
 * @brief      Creates a header.
 *
 * @param      header  The header
 *
 * @return     The dns header
 */
dns_header_t* create_header(dns_header_t* header) {
    uint16_t id = 1377;
    header->ID = htons(id);
    header->QR = 0;
    header->OP_CODE = 0;
    header-> AA = 0;
    header-> TC = 0;
    header-> RD = 0;
    header-> RA = 0;
    header-> Z = 0;
    header-> R_CODE = 0;
    header-> QDCOUNT = 0;
    header-> ANCOUNT = 0;
    header-> NSCOUNT = 0;
    header-> ARCOUNT = 0;
    return header;
}

/**
 * @brief      Creates a query message.
 *
 * @param      query_name  The query name
 *
 * @return     The query message
 */
query_message_t* create_query_message(char* query_name) {
    query_message_t* query_message;
    char encode_name[MAXLINE];
    memset(encode_name, 0, MAXLINE);

    query_message = (query_message_t*) malloc (sizeof(query_message_t));
    dns_header_t* header = create_header(&(query_message->header));
    header->QDCOUNT = htons(1);
    encode_domain(query_name, encode_name);

    query_message->question.QNAME = (char*)malloc(sizeof(char)* strlen(encode_name) + 1);
    memset(query_message->question.QNAME, 0, strlen(encode_name) + 1);

    memcpy(query_message->question.QNAME, encode_name, strlen(encode_name) + 1);
    query_message->question.QTYPE = htons(1);
    query_message->question.QCLASS = htons(1);
    return query_message;
}

/**
 * @brief      Creates an answer message.
 *
 * @param      response_ip    The response ip
 * @param      query_message  The query message
 *
 * @return     The answer message
 */
answer_message_t* create_answer_message(char* response_ip, query_message_t* query_message) {
    answer_message_t* answer_message = (answer_message_t*) malloc (sizeof(answer_message_t));
    dns_header_t* header = create_header(&(answer_message->header));
    header->ID = htons(query_message->header.ID);
    header->QR = 1;
    header->AA = 1;
    header-> ANCOUNT = htons(1);
    header-> QDCOUNT = htons(1);
    answer_message->answer.NAME = query_message->question.QNAME;
    answer_message->answer.TYPE = htons(1);
    answer_message->answer.CLASS = htons(1);
    answer_message->answer.TTL = htonl(0);
    answer_message->answer.RDLENGTH = htons(4);
    answer_message->answer.RDATA = (uint32_t)inet_addr(response_ip);

    return answer_message;
}

/**
 * @brief      Creates an error message.
 *
 * @param[in]  error          The error
 * @param      query_message  The query message
 *
 * @return     THe error message
 */
answer_message_t* create_error_message(int error, query_message_t* query_message) {
    answer_message_t* answer_message = (answer_message_t*) malloc (sizeof(answer_message_t));
    dns_header_t* header = create_header(&(answer_message->header));
    header->ID = htons(query_message->header.ID);
    header->QR = 1;
    header->AA = 1;
    header-> ANCOUNT = 0;
    header-> R_CODE = error; 
    return answer_message;
}

/**
 * @brief      Custmer funtion to adjust the order of the uint4 to network seq
 *
 * @param      buffer  The buffer
 */
void custom_hton4(char * buffer) {
    uint16_t ptr = *buffer;
    uint16_t leftmask = 0b1100110011001100;
    uint16_t rightmask = 0b0011001100110011;
    uint16_t tmp = ((ptr & leftmask) >> 2) | ((ptr & rightmask) << 2);
    memcpy(buffer, &tmp, 2);
}

/**
 * @brief      put the question into buffer
 *
 * @param      buffer         The buffer
 * @param      query_message  The query message
 */
void buffer_dns_question(char*buffer, query_message_t* query_message) {
    char* ptr = buffer;
    int len = sizeof(query_message->header);
    memcpy(buffer, &(query_message->header), len);
    custom_hton4(buffer+2);
    ptr += len;
    
    len = strlen(query_message->question.QNAME) + 1;
    memcpy(ptr, query_message->question.QNAME, len);
    ptr += len;

    len = sizeof(uint16_t);
    memcpy(ptr, &(query_message->question.QTYPE), sizeof(uint16_t));
    ptr += len;

    len = sizeof(uint16_t);
    memcpy(ptr, &(query_message->question.QCLASS), sizeof(uint16_t));
    ptr += len;
}

/**
 * @brief      put the answer into buffer
 *
 * @param      buffer          The buffer
 * @param      answer_message  The answer message
 */
void buffer_dns_answer(char*buffer, answer_message_t* answer_message) {
    char* ptr = buffer;
    int len = sizeof(answer_message->header);
    memcpy(buffer, &(answer_message->header), len);
    custom_hton4(buffer+2);
    ptr += len;

    len = strlen(answer_message->answer.NAME)+1;
    memcpy(ptr, answer_message->answer.NAME, len);
    ptr += len;

    len = sizeof(uint16_t);
    uint16_t TYPE = htons(1), CLASS = htons(1);

    memcpy(ptr, &TYPE, len);
    ptr += len;

    memcpy(ptr, &CLASS, len);
    ptr += len;

    uint16_t c00c = htons(49164);
    memcpy(ptr, &c00c, len);
    ptr += len;

    len = sizeof(uint16_t);
    memcpy(ptr, &(answer_message->answer.TYPE), len);
    ptr += len;

    len = sizeof(uint16_t);
    memcpy(ptr, &(answer_message->answer.CLASS), len);
    ptr += len;
    
    len = sizeof(uint32_t);
    memcpy(ptr, &(answer_message->answer.TTL), len);
    ptr += len;

    len = sizeof(uint16_t);
    memcpy(ptr, &(answer_message->answer.RDLENGTH), len);
    ptr += len;

    len = sizeof(uint32_t);
    uint32_t ip = answer_message->answer.RDATA;
    memcpy(ptr, &ip, len);
    ptr += len;

}

/**
 * @brief      Put the error response into buffer
 *
 * @param      buffer         The buffer
 * @param      error_message  The error message
 */
void buffer_dns_error(char*buffer, answer_message_t* error_message) {
    char* ptr = buffer;
    int len = sizeof(error_message->header);
    memcpy(buffer, &(error_message->header), len);
    custom_hton4(buffer+2);
}

/**
 * @brief      parse out the answer from buffer
 *
 * @param      buffer  The buffer
 *
 * @return     { description_of_the_return_value }
 */
answer_message_t* de_buffer_answer(char* buffer) {
    answer_message_t* answer_message = (answer_message_t*)malloc(sizeof(answer_message_t));
    memcpy(&(answer_message->header), buffer, sizeof(dns_header_t));
    custom_hton4((char*)&(answer_message->header)+2);
    dns_header_t* header = &(answer_message->header);
    header->ID = ntohs(header->ID);
    header->QDCOUNT = ntohs(header->ID);

    resource_record_t* answer = &(answer_message->answer);
    char* p = buffer + sizeof(answer_message->header);
    
    int len = strlen(p) + 1;
    answer->NAME = malloc(len);
    memcpy(answer->NAME, p, len);
    p += len + 6; // ignore question type, class and c00c
    len = sizeof(uint16_t);
    memcpy(&(answer->TYPE), p, sizeof(uint16_t));
    p += len;
    len = sizeof(uint16_t);
    memcpy(&(answer->CLASS), p, sizeof(uint16_t));
    p += len;
    len = sizeof(uint32_t);
    memcpy(&(answer->TTL), p, sizeof(uint32_t));
    p += len;
    len = sizeof(uint16_t);
    memcpy(&(answer->RDLENGTH), p, sizeof(uint16_t));
    p += len;
    len = sizeof(uint32_t);
    memcpy(&(answer->RDATA), p, sizeof(uint32_t));
    p += len;

    answer->TYPE = ntohs(answer->TYPE);
    answer->CLASS = ntohs(answer->CLASS);
    answer->TTL = ntohl(answer->TTL);
    answer->RDLENGTH = ntohs(answer->RDLENGTH);
    answer->RDATA = ntohl(answer->RDATA);

    printf("NAME is %s\n", answer->NAME);
    printf("TYPE is %d\n", answer->TYPE);
    printf("CLASS is %d\n", answer->CLASS);
    printf("TTL is %d\n", answer->TTL);
    printf("RDLENGTH is %d\n", answer->RDLENGTH);
    printf("RDATA is %x\n", answer->RDATA);

    return answer_message;
}

/**
 * @brief      Parse out the query from buffer
 *
 * @param      buffer  The buffer
 *
 * @return     The query message
 */
query_message_t* de_buffer_query(char* buffer) {
    /* start to copy header */
    query_message_t* query_message = (query_message_t*)malloc(sizeof(query_message_t));
    memcpy((char*)&(query_message->header), buffer, sizeof(dns_header_t));
    custom_hton4((char*)&(query_message->header)+2);

    dns_header_t* header = &(query_message->header);
    header->ID = ntohs(header->ID);
    header->QDCOUNT = ntohs(header->QDCOUNT);

    /* start to copy question */
    char* p = buffer + sizeof(query_message->header);
    dns_question_t* question = &(query_message->question);

    int len = strlen(p) + 1;
    question->QNAME = malloc(len);
    memcpy(question->QNAME, p, len);
    p += len;

    len = sizeof(uint16_t);
    memcpy(&(question->QTYPE), p, sizeof(uint16_t));
    p += len;

    len = sizeof(uint16_t);
    memcpy(&(question->QCLASS), p, sizeof(uint16_t));
    p += len;

    question->QTYPE = ntohs(question->QTYPE);
    question->QCLASS = ntohs(question->QCLASS);

    printf("QNAME has a length of %d\n", strlen(question->QNAME));
    printf("QNAME is %s\n", question->QNAME);
    printf("QTYPE is %hhx\n", question->QTYPE);
    printf("QCLASS is %hhx\n", question->QCLASS);

    return query_message;
}

/**
 * @brief      Parse out the error from buffer
 *
 * @param      buffer  The buffer
 *
 * @return     The error message
 */
answer_message_t* de_buffer_error(char* buffer) {
    answer_message_t* answer_message = (answer_message_t*)malloc(sizeof(answer_message_t));
    custom_hton4((char*)&(answer_message->header)+2);
    return answer_message;
}

/**
 * @brief      Encode the hostname 
 *
 * @param      domain_name  The domain name
 * @param      res_buf      The resource buffer
 */
void encode_domain(char* domain_name, char* res_buf) {
    // video.cs.cmu.edu -> 5video2cs3cmu3edu0
    printf("To be encoded string is %s", domain_name);
    char domain_cpy[4096];
    memset(domain_cpy, 0, 4096);
    strcpy(domain_cpy, domain_name);
    char* curr_head = domain_cpy;
    char* curr_end = strstr(curr_head, ".");
    while (curr_end != NULL) {
        *curr_end = '\0';
        char num_str[10];
        memset(num_str, 0, 10);
        sprintf(num_str, "%c", (int)strlen(curr_head));
        strcat(res_buf, num_str);
        strcat(res_buf, curr_head);
        *curr_end = '.';
        curr_head = curr_end + 1;
        curr_end = strstr(curr_head, ".");
    }
    char num_str[10];
    memset(num_str, 0, 10);
    sprintf(num_str, "%c", (int)strlen(curr_head));
    strcat(res_buf, num_str);
    strcat(res_buf, curr_head);
    char final[10];
    memset(final, 0, 10);
    sprintf(final, "%c", 0);
    strcat(res_buf, final);
}


/**
 * @brief      Decode the hostname
 *
 * @param      name  The name
 * @param      des   The description
 */
void decode_domain(char *name, char *des) {
    // 5video2cs3cmu3edu0 -> video.cs.cmu.edu
    int i = 0, j;
    char cnt;
    int len = strlen(name);
    while (i < len) {
        cnt = name[i];
        if (cnt == 0) {
            des[i - 1] = 0;
            break;
        }
        for (j = 0; j < cnt; j++) {
            des[i + j] = name[i + j + 1];
        }
        des[i + j] = '.';
        i += cnt + 1;
    }
    des[i - 1] = 0;
}

/**
 * @brief      Reads a servers ip.
 *
 * @param      servers_ip_file  The servers ip file
 * @param      graph            The graph
 */
void read_servers_ip(char* servers_ip_file, graph_t* graph) {
    FILE* file = fopen(servers_ip_file, "r");
    char line[MAXLINE];
    int count = 0;
    if (file == NULL) {
        printf("[Error] Error in open file %s\n", servers_ip_file);
        return;
    }
    while (fgets(line, MAXLINE, file)) {
        int len = strlen(line);
        line[len - 1] = '\0';
        printf("One of the server is %s\n", line);
        strcpy(graph->servers[count++], line);
    }
    graph->server_num = count;
}

/**
 * @brief      Determines if server.
 *
 * @param      name   The name
 * @param      graph  The graph
 *
 * @return     True if server, False otherwise.
 */
int is_server(char* name, graph_t* graph) {
    int i;
    for (i = 0; i < graph->server_num; ++i) {
        if (!strcmp(graph->servers[i], name)) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief      Adds a node.
 *
 * @param      graph    The graph
 * @param      name     The name
 * @param[in]  version  The version
 *
 * @return     { description_of_the_return_value }
 */
int add_node(graph_t* graph, char* name, int version) {
    graph_node_t* curr_node = &(graph->nodes[graph->size]);
    curr_node->name = malloc(MAXLINE);
    strcpy(curr_node->name, name);
    curr_node->version = version;
    graph->size++;
    curr_node->is_server = is_server(name, graph);
    return graph->size - 1;
}

/**
 * @brief      FInd the node 
 *
 * @param      graph       The graph
 * @param      name        The name
 * @param[in]  create_new  The create new
 *
 * @return     The index of the node
 */
int find_node(graph_t* graph, char* name, int create_new) {
    int i;
    for (i = 0; i < graph->size; ++i) {
        if (!strcmp(graph->nodes[i].name, name)) {
            return i;
        }
    }
    if (create_new) {
        return add_node(graph, name, 1);
    }
    return -1;
}

/**
 * @brief      print a node
 *
 * @param      graph  The graph
 * @param      node   The node
 */
void print_node(graph_t* graph, graph_node_t* node) {
    int i;
    printf("Current node is %s\n", node->name);
    for (i = 0; i < node->neighbor_num; ++i) {
        printf("Neighbor %d is %s\n", i, graph->nodes[node->neighbor_position[i]].name);
    }
}

/**
 * @brief      print the graph
 *
 * @param      graph  The graph
 */
void print_graph(graph_t* graph) {
    int i;
    printf("%s\n", "--------------------------");
    for (i = 0; i < graph->size; ++i) {
        print_node(graph, &(graph->nodes[i]));
    }
}

/**
 * @brief      Reads a lsa.
 *
 * @param      lsa_path  The lsa path
 * @param      graph     The graph
 */
void read_LSA(char* lsa_path, graph_t* graph) {
    FILE* file = fopen(lsa_path, "r");
    char* head_of_line = NULL;
    char line[MAXLINE];
    char* name;
    char* p;
    char* neighbor;
    int seq;
    int idx;

    graph->size = 0;
    graph_node_t* curr_node;
    int count;
    // graph_node_t nei_node;

    if (file == NULL) {
        printf("[Error] Error in open file %s\n", lsa_path);
        return;
    }

    while (fgets(line, MAXLINE, file)) {
        int len = strlen(line);
        line[len - 1] = '\0';
        p = strtok(line, " ");
        name = p;

        p = strtok(NULL, " ");
        seq = atoi(p);

        idx = find_node(graph, name, 1);
        curr_node = &(graph->nodes[idx]);
        if (curr_node->version > seq) {
            continue;
        }
        curr_node->version = seq;

        p = strtok(NULL, " ");
        neighbor = strtok(p, ",");
        count = 0;
        while (neighbor != NULL) {
            // printf("neighbor is %s\n", neighbor);
            idx = find_node(graph, neighbor, 1);
            curr_node->neighbor_position[count++] = idx;
            neighbor = strtok(NULL, ",");
        }
        curr_node->neighbor_num = count;
    }
    print_graph(graph);
    fclose(file);
}


/**
 * @brief      Conducting bfs
 *
 * @param      graph            The graph
 * @param      client           The client
 * @param      resolved_server  The resolved server
 */
void bfs(graph_t* graph, char* client, char* resolved_server) {
    int is_visited[graph->size];
    int path[graph->size];
    int nv = 0, nf = 1;
    int curr_node;
    int i;
    int client_idx = find_node(graph, client, 0);
    if (client_idx < 0) {
        printf ("[Error] Cannot find node %s", client);
        return;
    }
    // clear the memory default settings
    for (i = 0; i < graph->size; i++) {
        is_visited[i] = 0;
    }
    path[0] = client_idx;
    while (nv < nf) {
        curr_node = path[nv++];
        is_visited[curr_node] = 1;
        for (i = 0; i < graph->nodes[curr_node].neighbor_num; i++) {
            int curr_neighbor = graph->nodes[curr_node].neighbor_position[i];
            if (! is_visited[curr_neighbor]) {
                path[nf++] = curr_neighbor;
            }
            if (is_server(graph->nodes[curr_neighbor].name, graph)) {
                strcpy(resolved_server, graph->nodes[curr_neighbor].name);
                return;
            }
        }
    }
}


/**
 * @brief      Conducting dijkstra
 *
 * @param      graph            The graph
 * @param      client           The client
 * @param      resolved_server  The resolved server
 */
void dijkstra(graph_t* graph, char* client, char* resolved_server) {
    int cost[graph->size][graph->size];
    int is_visited[graph->size];
    int distance[graph->size];
    int i, j;
    int nv = 0;
    int curr_idx;
    int minDistance = 1 << 30;
    int minDisIndex = -1;
    for (i = 0; i < graph->size; i++) {
        distance[i] = 1 << 30;
        is_visited[i] = 0;
    }
    // predefine as max int
    for (i = 0; i < graph->size; i++) {
        for (j = 0; j < graph->size; j++) {
            cost[i][j] = 1 << 30;
        }
    }
    // build cost matrix
    for (i = 0; i < graph->size; i++) {
        for (j = 0; j < graph->nodes[i].neighbor_num; j++) {
            int curr_neighbor = graph->nodes[i].neighbor_position[j];
            cost[i][curr_neighbor] = 1;
            cost[curr_neighbor][i] = 1;
        }
    }


    int client_idx = find_node(graph, client, 0);
    if (client_idx < 0) {
        printf ("[Error] Cannot find node %s", client);
        return;
    }

    distance[client_idx] = 0;

    while (nv < graph->size) {
        minDistance = 1 << 30;
        // find the min distance node also not found already
        for (i = 0; i < graph->size; ++i) {
            if (distance[i] < minDistance && (!is_visited[i])) {
                minDistance = distance[i];
                minDisIndex = i;
            }
        }

        // if find a server, return it immediately
        if (is_server(graph->nodes[minDisIndex].name, graph)) {
            strcpy(resolved_server, graph->nodes[minDisIndex].name);
            return;
        }

        is_visited[minDisIndex] = 1;
        nv++;

        for (i = 0; i < graph->nodes[minDisIndex].neighbor_num; i++) {
            int curr_neighbor = graph->nodes[minDisIndex].neighbor_position[i];
            distance[curr_neighbor] = MIN(distance[curr_neighbor], cost[minDisIndex][curr_neighbor] + distance[minDisIndex]);
        }
    }
}
