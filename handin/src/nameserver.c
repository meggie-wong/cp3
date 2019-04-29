#include "nameserver.h"

#define MAXLINE 1024 

char* log_path;
char* dns_ip;
int dns_port;
char* servers_path;
char* LSAs_path;
int method_robin = 0;

void start_dns_server() {
	int sockfd; 
	char buffer[MAXLINE]; 
	char *hello = "3.0.0.1";
    char response[MAXLINE];  
	struct sockaddr_in servaddr, cliaddr; 
	
	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr =  inet_addr(dns_ip); // 打印的时候可以调用inet_ntoa()函数将其转换为char *类型.
	servaddr.sin_port = htons(dns_port); 
	
	// Bind the socket with the server address 
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
			sizeof(servaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	while(1) {
        int len, n; 
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                    &len); 
        buffer[n] = '\0'; 
        printf("Client : %s\n", buffer); 
        sendto(sockfd, (const char *)hello, strlen(hello), 
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
                len); 
        printf("DNS resolution sent.\n"); 
    }
	
}
int main(int argc, char* argv[]) {
    // start_proxying();
    printf("Starting the dns server...\n");
    int start_index = 0;

    if (argc != 6 && argc != 7) {
        printf("[Usage] ./nameserver [-r] <log> <ip> <port> <servers> <LSAs>\n");
        printf("provide %d args now\n", argc);
        return -1;
    }
    if (argc == 7) {
        start_index = 1;
        if (strstr(argv[1], "-r") == NULL) {
            printf("first argument should be -r\n");
            return -1;
        }
        method_robin = 1;
    } 
    
    log_path = argv[start_index+1];
    dns_ip = argv[start_index+2];
    dns_port = atoi(argv[start_index+3]);
    servers_path = argv[start_index+4];
    LSAs_path = argv[start_index+5];
    // init_log(log_path);
    start_dns_server();
    return 0;
}