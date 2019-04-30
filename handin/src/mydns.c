#include "mydns.h"

#define MAXLINE 1024

int client_fd;
struct sockaddr_in ser_addr, cli_addr;


int init_mydns(const char *dns_ip, unsigned int dns_port, const char* fake_ip) {

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_fd < 0)
    {
       printf("create socket fail!\n");
       return -1;
    }
    //绑定地址信息
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(9999);
    cli_addr.sin_addr.s_addr = inet_addr(fake_ip);
    if ( bind(client_fd, (struct sockaddr* )&cli_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    //ser_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    ser_addr.sin_addr.s_addr = inet_addr(dns_ip);  //注意网络序转换
    ser_addr.sin_port = htons(dns_port);  //注意网络序转换
    // udp_msg_sender(client_fd, (struct sockaddr*)&ser_addr);
    // close(client_fd);

    
}

int resolve(const char *node, const char *service, 
            const struct addrinfo *hints, struct addrinfo **res){

    // socklen_t len;
    struct sockaddr_in src;
    int n, len; 
    char buffer[MAXLINE]; 
    char *hello = "Hello from client";
    while(1)
    {
        sendto(client_fd, (const char *)hello, strlen(hello), 
                MSG_CONFIRM, (const struct sockaddr *) &ser_addr,  
                    sizeof(ser_addr)); 
        printf("Hello message sent.\n");

        memset(buffer, 0, MAXLINE);
        n = recvfrom(client_fd, (char *)buffer, MAXLINE,  
                MSG_WAITALL, (struct sockaddr *) &ser_addr, 
                &len); 
        buffer[n] = '\0'; 
        printf("Server : %s\n", buffer); 
        printf("S_ip = %s\n", inet_ntoa(ser_addr.sin_addr));
        printf("C_ip = %s\n", inet_ntoa(cli_addr.sin_addr));
        break;
    }
            
}