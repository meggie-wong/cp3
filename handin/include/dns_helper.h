#define MAXLINE 4096

struct dns_header {
    uint16_t ID;
    uint16_t QR : 1;
    uint16_t OP_CODE : 1;
    uint16_t AA : 1;
    uint16_t TC : 1;
    uint16_t RD : 1;
    uint16_t RA : 1;
    uint16_t RA : 3;
    uint16_t R_CODE : 4;
    uint16_t QDCOUNT;
    uint16_t ANCOUNT;
    uint16_t NSCOUNT;
    uint16_t ARCOUNT;
};

typedef dns_header dns_header_t;

struct question {
    char* QNAME;
    uint16_t QTYPE;
    uint16_t QCLASS;
};

typedef question dns_question_t;


struct resource_record {
    char* NAME
    uint16_t TYPE;
    uint16_t CLASS;
    uint32_t TTL;
    unint16_t RDLENGTH;
    char* RDATA;
};

typedef resource_record resource_record_t;


