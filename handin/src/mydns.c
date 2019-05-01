#include "mydns.h"

int client_fd;
struct sockaddr_in ser_addr, cli_addr;

char* get_response_ip(char* recv_buf, int n, char* response_ip) {
    answer_message_t* answer_message = de_buffer_answer(recv_buf);
    uint32_t ip = answer_message->answer.RDATA;
    printf("ip: %x\n", ip);
    struct in_addr ip_addr;
    ip_addr.s_addr = ip;
    sprintf(response_ip, "%s", inet_ntoa(ip_addr));
    printf("receive a ip of %s\n", response_ip);
    // TODO 完整性检验
    return response_ip;
}
int init_mydns(const char *dns_ip, unsigned int dns_port, const char* fake_ip) {

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_fd < 0)
    {
       printf("create socket fail!\n");
       return -1;
    }
    // set time out
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv)) {
        perror("setsockopt"); 
    }

    memset(&cli_addr, 0, sizeof(cli_addr));
    //bind client address
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(9999);
    cli_addr.sin_addr.s_addr = inet_addr(fake_ip);
    bzero(&cli_addr.sin_zero,8);
    // bind socket with addrinfo
    if ( bind(client_fd, (struct sockaddr* )&cli_addr, sizeof(struct sockaddr_in)) < 0)
    {
        printf("bind socket fail");
        exit(EXIT_FAILURE);
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(dns_ip);  
    ser_addr.sin_port = htons(dns_port); 
    bzero(&ser_addr.sin_zero,8);
    // close(client_fd);
}

int resolve(const char *query_name, char* response_ip){

    // socklen_t len;
    struct sockaddr_in src;
    int n, len; 
    char buffer[MAXLINE]; 
    char recv_buffer[MAXLINE];
    
    memset(buffer, 0, MAXLINE);
    query_message_t* query_message = create_query_message(query_name);
    buffer_dns_question(buffer, query_message);
    
    // char *hello = "Hello from client";
    while(1)
    {
        // int query_len = strlen(query_message->question.QNAME) + 1 + sizeof(query_message->header) + 4;

        // sendto(client_fd, (const char *)buffer, query_len, 
        //         MSG_CONFIRM, (const struct sockaddr *) &ser_addr,  
        //             sizeof(ser_addr)); 
        // printf("Query message sent.\n");

        // memset(recv_buffer, 0, MAXLINE);
        // n = recvfrom(client_fd, (char *)recv_buffer, MAXLINE,  
        //         0, (struct sockaddr *) &ser_addr, 
        //         &len); 
        // recv_buffer[n] = '\0'; 
        // // TODO get response ip and memcpy to response_ip  **cornercase** 
        // // recv length is shorter than expected
        // // recv format is invalid
        // printf("Server : %s\n", recv_buffer); 
        // printf("S_ip = %s\n", inet_ntoa(ser_addr.sin_addr));
        // printf("C_ip = %s\n", inet_ntoa(cli_addr.sin_addr));
        // break;

        int query_len = strlen(query_message->question.QNAME) + 1 + sizeof(query_message->header) + 4;

        sendto(client_fd, (const char *)buffer, query_len, 
                MSG_CONFIRM, (const struct sockaddr *) &ser_addr,  
                    sizeof(ser_addr)); 
        printf("Query message sent.\n");

        memset(recv_buffer, 0, MAXLINE);
        n = recvfrom(client_fd, (char *)recv_buffer, MAXLINE,  
                0, (struct sockaddr *) &ser_addr, 
                &len); 
        recv_buffer[n] = '\0';
        printf("receive %d bytes of data\n", n);
        // TODO get response ip and memcpy to response_ip  **cornercase**
        answer_message_t* answer_message = de_buffer_answer(recv_buffer);
        uint32_t ip = htonl(answer_message->answer.RDATA);

        printf("get ip: %x\n", ip);
        struct in_addr ip_addr;
        ip_addr.s_addr = ip;
        sprintf(response_ip, "%s",inet_ntoa(ip_addr));
        printf("receive a ip of %s\n", response_ip);
        
        // recv length is shorter than expected
        // recv format is invalid
        printf("Server : %s\n", recv_buffer);
        printf("S_ip = %s\n", inet_ntoa(ser_addr.sin_addr));
        printf("C_ip = %s\n", inet_ntoa(cli_addr.sin_addr));
        break;
    }
            
}