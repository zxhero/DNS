#ifndef DNS_PROTOCAL
#define DNS_PROTOCAL

#include <arpa/inet.h>
#include<string.h>

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

//Type
#define A_          1
#define MX_         15
#define CNAME_      5
//Class
#define IN_         1

typedef struct dns_header_t{
    unsigned short  id;
    unsigned short  tag;
    unsigned short  ques_count;
    unsigned short  ans_count;
    unsigned short  auth_count;
    unsigned short  add_count;
}dns_header;

#define DNS_HEADER_SIZE sizeof(struct dns_header_t)
typedef struct dns_query_t{
    unsigned char *dormain_name;
    unsigned short  qtype;
    unsigned short  qclass;
}dns_query;

typedef struct db_entry_t{
    unsigned char*  domain_name;
    unsigned short  type;
    unsigned short  _class;
    unsigned int  ttl;
    unsigned short  length;
    unsigned char*  data;
}db_entry;

void    init_dns_header(dns_header *dns, unsigned short id, unsigned int error_code_, int is_query, unsigned short ques_count, unsigned short ans_count, unsigned short add_count){
    dns->add_count = htons(add_count);
    dns->ans_count = htons(ans_count);
    dns->ques_count = htons(ques_count);
    dns->id = htons(id);
    if(is_query){
        dns->tag = htons(QR_query);
    }
    else{
        dns->tag = htons(QR_response | error_code_);
    }
}

unsigned int get_domain_name_len(unsigned char *ptr){
    unsigned int length = 0;
    while(*ptr != '\0'){
        length++;
        ptr++;
    }
    return length + 1;
}

int hton_domain_name(unsigned char* buff, unsigned char* src){
    unsigned char* name_next = src;
   // unsigned char* buff[100], *dest = buff;
    unsigned char *dest = buff;
    unsigned int len = 0;
    while(*name_next != '\0'){
        unsigned char length = 0;
        while(*name_next != '.' && *name_next != '\0'){
            length +=3;
            name_next +=3;
        }
        //name_next++;
        *dest++ = length;
        len += (length+1);
        while(src != name_next){
            *dest++ = *src++;
        }
        len++;
        if(*name_next == '\0'){
            *dest = '\0';
            break;
        }
        else{
            *dest++ = *src++;
            name_next++;
        }
    }
    return len;
    //unsigned char* n_domain_name = malloc(len);
    //memcpy(n_domain_name, buff, len);
    //return n_domain_name;
}

struct dns_query_t* get_ques_section(dns_header *dns){
    unsigned char* name = (unsigned char*)dns + DNS_HEADER_SIZE;
    unsigned int length = get_domain_name_len(name);
    struct dns_query_t* qsection = malloc(sizeof(struct dns_query_t));
    qsection->dormain_name = name;
    qsection->qtype = ntohs(*((unsigned short*)(name+length)));
    qsection->qclass = ntohs(*((unsigned short*)(name+length) + 1));
    return qsection;
}

void init_ques_section(dns_header *dns, unsigned char *domain_name, unsigned short type, unsigned short qclass){
    unsigned char* name = (unsigned char*)dns + DNS_HEADER_SIZE;
    name += hton_domain_name(name,domain_name);
    unsigned short * data = (unsigned short *)name;
    *data++ = htons(type);
    *data = htons(qclass);
}

struct db_entry_t* get_rr_entry(unsigned char* rr_begin){
    unsigned int length = get_domain_name_len(rr_begin);
    struct db_entry_t* rr_entry = malloc(sizeof(struct db_entry_t));
    rr_entry->data = rr_begin + length + 3 * sizeof(unsigned short) + sizeof(unsigned int);
    rr_entry->domain_name = rr_begin;
    rr_entry->ttl = ntohl(*((unsigned int*)(rr_begin+length + 2 * sizeof(unsigned short))));
    rr_entry->type = ntohs(*((unsigned short*)(rr_begin+length)));
    rr_entry->length = ntohs(*((unsigned short*)(rr_begin+length+ 2 * sizeof(unsigned short) + sizeof(unsigned int))));
    rr_entry->_class = ntohs(*((unsigned short*)(rr_begin+length) + 1));
    return rr_entry;
}

void init_rr_section(unsigned char* rr_begin, struct db_entry_t* rr){
    rr_begin += hton_domain_name(rr_begin,rr->domain_name);
    *(unsigned short*)rr_begin = htons(rr->type);
    rr_begin += sizeof(unsigned short);
    *(unsigned short*)rr_begin = htons(rr->_class);
    rr_begin += sizeof(unsigned short);
    *(unsigned int*)rr_begin = htonl(rr->ttl);
    rr_begin += sizeof(unsigned int);
    *(unsigned short*)rr_begin = htons(rr->length);
    rr_begin += sizeof(unsigned short);
    memcpy(rr_begin,rr->data,rr->length);
}

unsigned char *get_last_field_name(unsigned char *name){
    unsigned char *name_next = name;
    while(*name_next != '\0'){
        while(*name_next != '.' && *name_next != '\0') name_next++;
        if(*name_next != '\0'){
            name_next++;
            name = name_next;
        }
    }
    return name;
}

unsigned char *get_prior_field_name(unsigned char *name, unsigned char* name_head){
    if(name == name_head)   return NULL;
    unsigned char *name_prior = name - 2;
    while(*name_prior != '.' && name_prior != name_head){
        name_prior--;
    }
    if(name_prior == name_head) return name_prior;
    else    return name_prior+1;
}
#endif // DNS_PROTOCAL
