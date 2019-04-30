#include "mydns.h"


int client_fd;
struct sockaddr_in ser_addr, cli_addr;


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

int resolve(const char *query_name, const char *service, 
            const struct addrinfo *hints, struct addrinfo **res){

    // socklen_t len;
    struct sockaddr_in src;
    int n, len; 
    char buffer[MAXLINE]; 
    char recv_buffer[MAXLINE];
    
    memset(buffer, 0, MAXLINE);
    query_message_t* query_message = create_query_message(query_name);
    buffer_dns_question(buffer, query_message);
    
    char *hello = "Hello from client";
    while(1)
    {
        int i = 0;
        for(i = 0; i<strlen(query_message->question.QNAME) + sizeof(query_message)+10; i++) {
            printf("%d[%c] ", buffer[i], buffer[i]);
        }

        sendto(client_fd, (const char *)buffer, strlen(query_message->question.QNAME) + sizeof(query_message), 
                MSG_CONFIRM, (const struct sockaddr *) &ser_addr,  
                    sizeof(ser_addr)); 
        printf("Query message sent.\n");

        memset(recv_buffer, 0, MAXLINE);
        n = recvfrom(client_fd, (char *)recv_buffer, MAXLINE,  
                0, (struct sockaddr *) &ser_addr, 
                &len); 
        recv_buffer[n] = '\0'; 
        printf("Server : %s\n", recv_buffer); 
        printf("S_ip = %s\n", inet_ntoa(ser_addr.sin_addr));
        printf("C_ip = %s\n", inet_ntoa(cli_addr.sin_addr));
        break;
    }
            
}