#include "nameserver.h"

#define MAXLINE 1024 

char* log_path;
char* dns_ip;
int dns_port;
char* servers_path;
char* LSAs_path;
int method_robin = 0;
int res_cnt = 0;
dns_record_t dns_records = {.hostname = "video.cmu.cs.edu",.resolve_cnt = 0, .record_cnt = 0};


char* get_query_name(char* recv_buffer) {
    
    return NULL;

}

char* get_response_ip(char* query_name) {
    char * response_ip;
    if ( method_robin ) {
        if( strcmp(dns_records.hostname, query_name) == 0 ) { // is video.cmu.cs.edu 
            response_ip = dns_records.server_ip[dns_records.resolve_cnt%dns_records.record_cnt];
            dns_records.resolve_cnt ++;
            return response_ip;
        }

    } else {
        printf(" not use robin \n");

    }

}
void start_dns_server() {
	int sockfd; 
	char buffer[MAXLINE]; 
    char response[MAXLINE];  
	struct sockaddr_in servaddr, cliaddr; 

    int len, n; 
    char *response_ip = "3.0.0.1";
    char* client_ip;
    char* query_name;
    struct timeval now;
	
	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = inet_addr(dns_ip); // 打印的时候可以调用inet_ntoa()函数将其转换为char *类型.
	servaddr.sin_port = htons(dns_port); 
	
	// Bind the socket with the server address 
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
			sizeof(servaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	while(1) {
        
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                    &len); 
        buffer[n] = '\0'; 
        client_ip = inet_ntoa(cliaddr.sin_addr); 
        query_name = buffer;

        // query_name = get_query_name(buffer);
        // response_ip = get_response_ip(query_name);

        printf("Client : %s(ip=%s)\n", buffer, inet_ntoa(cliaddr.sin_addr)); 
        sendto(sockfd, (const char *)response_ip, strlen(response_ip), 
            0, (const struct sockaddr *) &cliaddr, 
                len);
        // write DNS log
        // <time> <client-ip> <query-name> <response-ip>
        gettimeofday(&now, NULL);
        logger("%ld %s %s %s\n", (long)now.tv_sec, client_ip, query_name, response_ip);

        printf("DNS resolution sent.\n"); 
    }
	
}
int main(int argc, char* argv[]) {
    // start_proxying();
    printf("Starting the dns server...\n");
    int start_index = 0;
    FILE *fp; 
	char strLine[MAXLINE];	

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
    init_log(log_path);
	if((fp = fopen(servers_path,"r")) == NULL)		
	{ 
		printf("Open %s Falied!", servers_path); 
		return -1; 
	} 
	while (!feof(fp))									
	{ 
        dns_records.server_ip[dns_records.record_cnt] = (char*)malloc(MAXLINE*sizeof(char));
		fgets(dns_records.server_ip[dns_records.record_cnt], MAXLINE, fp);					    
		printf("read %s", dns_records.server_ip[dns_records.record_cnt]);
		dns_records.record_cnt ++;
	} 
	fclose(fp);											//关闭文件
    start_dns_server();
    return 0;
}