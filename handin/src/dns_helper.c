// #include "dns_helper.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dns_helper.h"


dns_header_t* create_header() {
    dns_header_t* header = (dns_header_t*)malloc(sizeof(dns_header_t));
    header->ID = 1337;
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

query_message_t* create_query_message(char* query_name) {
    query_message_t* query_message;
    char encode_name[MAXLINE];
    dns_header_t* header = create_header();
    header->QR = 0;
    header->AA = 0;
    header-> QDCOUNT = htonl(1);
    query_message = (query_message_t*) malloc (sizeof(query_message_t));
    encode_domain(query_name, encode_name);
    query_message->question.QNAME = (char*)malloc(sizeof(char)* strlen(encode_name));
    strcpy(query_message->question.QNAME, encode_name);
    query_message->question.NMLENGTH = strlen(query_message->question.QNAME);
    query_message->question.QTYPE = htonl(1);
    query_message->question.QCLASS = htonl(1);
    return query_message;
}

answer_message_t* create_answer_message(char* response_ip, char* name) {
    answer_message_t* answer_message = (answer_message_t*) malloc (sizeof(answer_message_t));
    dns_header_t* header = create_header();
    header->QR = 1;
    header->AA = 1;
    header-> ANCOUNT = htonl(1);
    answer_message->answer.NMLENGTH = strlen(name);
    answer_message->answer.NAME = name;
    answer_message->answer.TYPE = htonl(1);
    answer_message->answer.CLASS = htonl(1);
    answer_message->answer.TTL = htonl(0);
    answer_message->answer.RDLENGTH = htonl(4);
    answer_message->answer.RDATA = (char*)malloc(4);
    unsigned_t ip = inet_addr(response_ip);
    memcpy(answer_message->answer.RDATA, ip, 4);
    return answer_message;
}

answer_message_t* create_error_message(char* response_ip, int error) {
    answer_message_t* answer_message = (answer_message_t*) malloc (sizeof(answer_message_t));
    dns_header_t* header = create_header();
    header->QR = 1;
    header->AA = 1;
    header-> ANCOUNT = 0;
    header-> R_CODE = error; 
    return answer_message;
}

void buffer_dns_question(char*buffer, query_message_t* query_message) {
    char* ptr = buffer;
    int len = sizeof(query_message->header);

    memcpy(buffer, &(query_message->header), len);
    ptr += len;

    len = strlen(query_message->question.QNAME);
    memcpy(ptr, &(query_message->question.QNAME), len);
    ptr += len;

    len = sizeof(uint16_t);
    memcpy(ptr, &(query_message->question.QTYPE), sizeof(uint16_t));
    ptr += len;

    len = sizeof(uint16_t);
    memcpy(ptr, &(query_message->question.QCLASS), sizeof(uint16_t));
    ptr += len;
}

void buffer_dns_answer(char*buffer, answer_message_t* answer_message) {
    char* ptr = buffer;
    int len = sizeof(answer_message->header);
    memcpy(buffer, &(answer_message->header), len);
    ptr += len;

    len = strlen(answer_message->answer.NAME);
    memcpy(ptr, &(answer_message->answer.NAME), len);
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

    len = strlen(answer_message->answer.RDATA);
    memcpy(ptr, answer_message->answer.RDATA, len);
    ptr += len;

}

void buffer_dns_error(char*buffer, answer_message_t* error_message) {
    char* ptr = buffer;
    int len = sizeof(error_message->header);
    memcpy(buffer, &(error_message->header), len);
}

void encode_domain(char* domain_name, char* res_buf) {
    // video.cs.cmu.edu -> 5video2cs3cmu3edu0
    // 先copy给定的domain name, 因为它是final的时候会cause bus error
    char domain_cpy[4096];
    strcpy(domain_cpy, domain_name);
    char* curr_head = domain_cpy;
    char* curr_end = strstr(curr_head, ".");
    while (curr_end != NULL) {
        // 将.暂时设为\0
        *curr_end = '\0';
        // 把int整形转换为字符串
        char num_str[10];
        memset(num_str, 0, 10);
        sprintf(num_str, "%c", (int)strlen(curr_head));
        // 把数字append上去
        strcat(res_buf, num_str);
        // 把原内容append上去
        strcat(res_buf, curr_head);
        // 把原地址改变的0设置回去
        *curr_end = '.';
        curr_head = curr_end + 1;
        curr_end = strstr(curr_head, ".");
    }
    // 最后一个循环找不到'.'，手动append剩下的内容
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


void decode_domain(char* domain_encode , char* res_buf) {
    // 5video2cs3cmu3edu0 -> video.cs.cmu.edu
    // not necessary to be implemented
}

int add_node(graph_t* graph, char* name, int version) {
    graph_node_t* curr_node = &(graph->nodes[graph->size]);
    curr_node->name = malloc(MAXLINE);
    strcpy(curr_node->name, name);
    curr_node->version = version;
    graph->size++;
    return graph->size - 1;
}

int find_node(graph_t* graph, char* name) {
    int i;
    for (i = 0; i < graph->size; ++i) {
        if (!strcmp(graph->nodes[i].name, name)) {
            return i;
        }
    }
    return add_node(graph, name, 1);
}

void print_node(graph_t* graph, graph_node_t* node) {
    int i;
    printf("Current node is %s\n", node->name);
    for (i = 0; i < node->neighbor_num; ++i) {
        printf("Neighbor %d is %s\n", i, graph->nodes[node->neighbor_position[i]].name);
    }
}

void print_graph(graph_t* graph) {
    int i;
    printf("%s\n", "--------------------------");
    for (i = 0; i < graph->size; ++i) {
        print_node(graph, &(graph->nodes[i]));
    }
}

void read_LSA(char* lsa_path) {
    FILE* file = fopen(lsa_path, "r");
    char* head_of_line = NULL;
    char line[MAXLINE];
    char* name;
    char* p;
    char* neighbor;
    int seq;
    int idx;
    graph_t* graph = malloc(sizeof(graph_t));
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

        // printf("Current sequence number %d start node is %s\n", seq, name);
        idx = find_node(graph, name);
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
            idx = find_node(graph, neighbor);
            curr_node->neighbor_position[count++] = idx;
            neighbor = strtok(NULL, ",");
        }
        curr_node->neighbor_num = count;
    }
    print_graph(graph);
    fclose(file);
}


// int main() {
// //    char res[100];
// //    memset(res, 0, 100);
// //    encode_domain("video.cs.cmu.edu", res);
// //    printf("%s\n", res);

//     read_LSA("topo2.lsa");
//     return 0;
// }