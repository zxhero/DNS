#ifndef DNS_PROTOCAL
#define DNS_PROTOCAL

//QR
#define QR_query 0x0000
#define QR_response 0x8000
//opcode
#define Stand_Query 0x0000
#define Inverse_Query 0x0800
#define Sever_Status_Requst 0x1000
//aa
#define Auth_Server 0x0400
//TC
#define Is_Trunction 0x0200
//RD
#define Recursive_Service 0x0100
//RA
#define Recursion_Avaliable 0x0080
//rcode
#define NO_ERROR            0x0000
#define Format_Error        0x0001
#define Server_Failure      0x0002
#define Name_Not_Exist      0x0003
#define Type_Not_Support    0x0004
#define Quary_Refused       0x0005

typedef struct dns_header_t{
    unsigned short  id;
    unsigned short  tag;
    unsigned short  ques_count;
    unsigned short  ans_count;
    unsigned short  auth_count;
    unsigned short  add_count;
}dns_header;

typedef struct dns_query_t{
    unsigned char *dormain_name;
    unsigned short  qtype;
    unsigned short  qclass;
}dns_query;

void    init_dns_header(){

}

typedef struct dns_query_t* get_ques_section(){

}

unsigned int get_domain_name_len(unsigned char *ptr){
    unsigned int length = 0;
    while(*ptr != '\0'){
        length++;
    }
    return length + 1;
}
#endif // DNS_PROTOCAL
