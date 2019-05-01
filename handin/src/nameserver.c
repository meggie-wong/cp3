#include "nameserver.h"

// #define MAXLINE 1024 

char* log_path;
char* dns_ip;
int dns_port;
char* servers_path;
char* LSAs_path;
int method_robin = 0;
int res_cnt = 0;
dns_record_t dns_records = {.hostname = "video.cs.cmu.edu",.resolve_cnt = 0, .record_cnt = 0};
graph_t* graph;

/**
 * @brief      Gets the query name.
 *
 * @param      query_message  The query message
 * @param      query_name     The query name
 */
void get_query_name(query_message_t* query_message, char * query_name) {
    decode_domain(query_message->question.QNAME, query_name);
}

/**
 * @brief      Gets the response ip.
 *
 * @param      query_name  The query name
 * @param      client_ip   The client ip
 *
 * @return     The response ip.
 */
char* get_response_ip(char* query_name, char* client_ip) {
    char * response_ip;
    if ( method_robin ) {
        if( strcmp(dns_records.hostname, query_name) == 0 ) { // is video.cmu.cs.edu 
            response_ip = dns_records.server_ip[dns_records.resolve_cnt%dns_records.record_cnt];
            dns_records.resolve_cnt ++;
            printf("is Valid domain name!!!\n");
            return response_ip;
        } else {
            printf("is inValid domain name!!!\n");
            return NULL;
        }
    } else {
        printf("Use dijkstra\n");
        if (strcmp(dns_records.hostname, query_name) == 0) {
            response_ip = malloc(MAXLINE);
            memset(response_ip, 0, MAXLINE);
            dijkstra(graph, client_ip, response_ip);
            printf("is Valid domain name!!!\n");
            return response_ip;
        } else {
            printf("is inValid domain name!!!\n");
            return NULL;
        }
    }
}

/**
 * @brief      Starts a dns server.
 */
void start_dns_server() {
	int sockfd; 
	char buffer[MAXLINE]; 
    char response[MAXLINE];  
	struct sockaddr_in servaddr, cliaddr; 

    int len = sizeof(struct sockaddr_in), n; 
    char *response_ip = "3.0.0.1";
    char* client_ip;
    char query_name[MAXLINE];
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
	servaddr.sin_addr.s_addr = inet_addr(dns_ip); 
	servaddr.sin_port = htons(dns_port); 
	
	// Bind the socket with the server address 
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
			sizeof(servaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 

	while(1) {
        answer_message_t* answer_message;
        int response_len = 0;
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                    &len);
        printf("received %d bytes in start dns server\n", n);
        buffer[n] = '\0'; 
        client_ip = inet_ntoa(cliaddr.sin_addr); 
        query_message_t* query_message = de_buffer_query(buffer);
        // printf("query name is %s after decode\n", query_message->question.QNAME);
        // int i = 0;
        // for (i = 0; i < 15; ++i) {
        //     printf("%x ", query_message->question.QNAME[i]);
        // }
        // printf("\n");
        get_query_name(query_message, query_name);
        printf("Client : %s(ip=%s)\n", query_name, inet_ntoa(cliaddr.sin_addr)); 
        printf("After recover the query name is %s\n", query_name);
        memset(buffer, 0, MAXLINE);
        response_ip = get_response_ip(query_name, client_ip); 
        if (response_ip != NULL) {
            answer_message = create_answer_message(response_ip, query_message);
            buffer_dns_answer(buffer, answer_message);
            response_len =  strlen(answer_message->answer.NAME) + 1 + sizeof(answer_message->header) + 20;  
        } else {
            answer_message = create_error_message(3, query_message);
            buffer_dns_error(buffer, answer_message);
            response_len = sizeof(answer_message->header);
        }
           
        sendto(sockfd, (const char *)buffer, response_len, 
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
                len);
        // write DNS log
        // <time> <client-ip> <query-name> <response-ip>
        gettimeofday(&now, NULL);
        logger("%ld %s %s %s\n", (long)now.tv_sec, client_ip, query_name, response_ip);
        printf("DNS resolution sent.\n"); 
    }
	
}
/**
 * @brief      Main function
 *
 * @param[in]  argc  The argc
 * @param      argv  The argv
 *
 * @return    DNS server main function
 */
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
        int len = strlen(dns_records.server_ip[dns_records.record_cnt]);
        if (len > 0) {
            dns_records.server_ip[dns_records.record_cnt][len] = '\0'; 
		    printf("read %s\n", dns_records.server_ip[dns_records.record_cnt]);
		    dns_records.record_cnt ++;
        }
	} 
	fclose(fp);				

    graph = malloc(sizeof(graph_t));
    read_servers_ip(servers_path, graph);
    read_LSA(LSAs_path, graph);
    start_dns_server();
    return 0;
}