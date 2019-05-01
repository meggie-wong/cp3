// #include "dns_helper.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dns_helper.h"


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

answer_message_t* create_answer_message(char* response_ip, char* name) {
    answer_message_t* answer_message = (answer_message_t*) malloc (sizeof(answer_message_t));
    dns_header_t* header = create_header(&(answer_message->header));
    header->QR = 1;
    header->AA = 1;
    header-> ANCOUNT = htons(1);
    header-> QDCOUNT = htons(1);
    answer_message->answer.NAME = name;
    answer_message->answer.TYPE = htons(1);
    answer_message->answer.CLASS = htons(1);
    answer_message->answer.TTL = htonl(0);
    answer_message->answer.RDLENGTH = htons(4);
    answer_message->answer.RDATA = (uint32_t)inet_addr(response_ip);
    // struct in_addr ip;
    // inet_aton(response_ip, &ip);   /* 将字符串转换为二进制 */
    // uint32_t s_ip = ip.s_addr;
    // answer_message->answer.RDATA = htonl(s_ip);
    return answer_message;
}

answer_message_t* create_error_message(int error) {
    answer_message_t* answer_message = (answer_message_t*) malloc (sizeof(answer_message_t));
    dns_header_t* header = create_header(&(answer_message->header));
    header->QR = 1;
    header->AA = 1;
    header-> ANCOUNT = 0;
    header-> R_CODE = error; 
    return answer_message;
}

void custom_hton4(char * buffer) {
    // int len = sizeof(*header);
    // memcpy(buffer, header, len);
    uint16_t ptr = *buffer;
    uint16_t leftmask = 0b1100110011001100;
    uint16_t rightmask = 0b0011001100110011;
    uint16_t tmp = ((ptr & leftmask) >> 2) | ((ptr & rightmask) << 2);
    memcpy(buffer, &tmp, 2);
}

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

void buffer_dns_answer(char*buffer, answer_message_t* answer_message) {
    char* ptr = buffer;
    int len = sizeof(answer_message->header);
    memcpy(buffer, &(answer_message->header), len);
    custom_hton4(buffer+2);
    ptr += len;

    len = strlen(answer_message->answer.NAME)+1;
    memcpy(ptr, answer_message->answer.NAME, len);
    ptr += len;

// ====== should have a question filed
    len = sizeof(uint16_t);
    uint16_t TYPE = htons(1), CLASS = htons(1);

    memcpy(ptr, &TYPE, len);
    ptr += len;

    memcpy(ptr, &CLASS, len);
    ptr += len;

    uint16_t c00c = htons(49164);
    memcpy(ptr, &c00c, len);
    ptr += len;
// ====== end

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
    // printf("ip = %x  %d\n", ip, len);
    memcpy(ptr, &ip, len);
    // printf("%x, %x, %x, %x", ptr[0], ptr[1], ptr[2], ptr[3]);
    ptr += len;
    int i = 0;
    for(i = 0; i < strlen(answer_message->answer.NAME) + sizeof(answer_message->header) + 15; i++) {
        // printf("%hhx ",buffer[i]);
        printf("%hhx[%c] ", buffer[i], buffer[i]);
    }
    printf("=======END\n");

}

void buffer_dns_error(char*buffer, answer_message_t* error_message) {
    char* ptr = buffer;
    int len = sizeof(error_message->header);
    memcpy(buffer, &(error_message->header), len);
    custom_hton4(buffer+2);
    // buffer_dns_header(buffer, &(error_message->header));
}

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
    printf("read %d name %s \n",len, p);
    answer->NAME = malloc(len);
    memcpy(answer->NAME, p, len);
    p += len + 3; // ignore question type, class and c00c
    printf("read answer %x %x\n",p[0], p[1]);
    len = sizeof(uint16_t);
    memcpy(&(answer->TYPE), p, sizeof(uint16_t));
    p += len;
    printf("read answer %x %x\n",p[0], p[1]);
    len = sizeof(uint16_t);
    memcpy(&(answer->CLASS), p, sizeof(uint16_t));
    p += len;
    printf("read answer %d\n",*p);
    len = sizeof(uint32_t);
    memcpy(&(answer->TTL), p, sizeof(uint32_t));
    p += len;
    printf("read answer %x %x\n",p[0], p[1]);
    len = sizeof(uint16_t);
    memcpy(&(answer->RDLENGTH), p, sizeof(uint16_t));
    p += len;
    printf("read rdate %d\n",*p);
    len = sizeof(uint32_t);
    memcpy(&(answer->RDATA), p, sizeof(uint32_t));
    p += len;

//    answer->TYPE = ntohs(answer->TYPE);
//    answer->CLASS = ntohs(answer->CLASS);
//    answer->TTL = ntohl(answer->TTL);
//    answer->RDLENGTH = ntohs(answer->RDLENGTH);
//    answer->RDATA = ntohl(answer->RDATA);

    printf("NAME is %s\n", answer->NAME);
    printf("TYPE is %d\n", answer->TYPE);
    printf("CLASS is %d\n", answer->CLASS);
    printf("TTL is %d\n", answer->TTL);
    printf("RDLENGTH is %d\n", answer->RDLENGTH);
    printf("RDATA is %x\n", answer->RDATA);

    return answer_message;
}



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
    int i = 0;
    for(i=0;i<15;i++){
        printf("%hhx ", p[i]);
    }
    printf("+++++ %hhx,%hhx\n", p[0], p[1]);

    int len = strlen(p) + 1;
    printf("has a intermediate length of %d\n", len);
    question->QNAME = malloc(len);
    memcpy(question->QNAME, p, len);
    p += len;

    len = sizeof(uint16_t);
    memcpy(&(question->QTYPE), p, sizeof(uint16_t));
    p += len;

    len = sizeof(uint16_t);
    memcpy(&(question->QCLASS), p, sizeof(uint16_t));
    p += len;

//    question->QTYPE = ntohs(question->QTYPE);
//    question->QCLASS = ntohs(question->QCLASS);


    printf("QNAME has a length of %d\n", strlen(question->QNAME));
    printf("QNAME is %s\n", question->QNAME);
    printf("QTYPE is %hhx\n", question->QTYPE);
    printf("QCLASS is %hhx\n", question->QCLASS);


    return query_message;
}

answer_message_t* de_buffer_error(char* buffer) {
    answer_message_t* answer_message = (answer_message_t*)malloc(sizeof(answer_message_t));
    custom_hton4((char*)&(answer_message->header)+2);
    return answer_message;
}

void encode_domain(char* domain_name, char* res_buf) {
    // video.cs.cmu.edu -> 5video2cs3cmu3edu0
    // 先copy给定的domain name, 因为它是final的时候会cause bus error
    printf("To be encoded string is %s", domain_name);
    char domain_cpy[4096];
    memset(domain_cpy, 0, 4096);
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


void decode_domain(char *name, char *des) {
    // 5video2cs3cmu3edu0 -> video.cs.cmu.edu
    // not necessary to be implemented
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