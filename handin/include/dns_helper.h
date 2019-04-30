#define MAXLINE 4096
#define MAX_NODE 1024

struct dns_header {
    uint16_t ID;
    uint16_t QR : 1;
    uint16_t OP_CODE : 1;
    uint16_t AA : 1;
    uint16_t TC : 1;
    uint16_t RD : 1;
    uint16_t RA : 1;
    uint16_t Z : 3;
    uint16_t R_CODE : 4;
    uint16_t QDCOUNT;
    uint16_t ANCOUNT;
    uint16_t NSCOUNT;
    uint16_t ARCOUNT;
};

typedef struct dns_header dns_header_t;

struct question {
    char* QNAME;
    uint16_t QTYPE;
    uint16_t QCLASS;
};

typedef struct question dns_question_t;


struct resource_record {
    char* NAME;
    uint16_t TYPE;
    uint16_t CLASS;
    uint32_t TTL;
    uint16_t RDLENGTH;
    char* RDATA;
};

typedef struct resource_record resource_record_t;

struct query_message {
    dns_header_t header;
    dns_question_t question;
};

typedef struct query_message query_message_t;


struct answer_message {
    dns_header_t header;
    resource_record_t answer;
};

typedef struct answer_message answer_message_t;

struct graph_node {
    char* name; // node name. eg. "router2, 3.0.0.1"
    int version; // the version number of the update for this node
    int index; // index-th node in the graph
    int is_server; // 1 is server, 0 is not server
    int neighbor_position[1024]; // use index to find all the neighbors
    int neighbor_num;
};

typedef struct graph_node graph_node_t;

struct graph_nodes {
    graph_node_t nodes[MAX_NODE];
    int size;
};

typedef struct graph_nodes graph_t;
