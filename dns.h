#ifndef DNS
#define DNS

#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include "dns_db.h"
#include "dns_protocal.h"

void    reply_dns_query(unsigned char *packet, db_entry *ans_section, db_entry *add_section, int error_code, int socketfd){
    dns_header *dns_h = (struct dns_header_t*)(packet);
    host_rr_to_net(ans_section);
    host_rr_to_net(add_section);
    unsigned short rply_length = 0;
    unsigned char * rply_packet = NULL;
    if(add_section == NULL && ans_section != NULL){
         rply_length = 2 + DNS_HEADER_SIZE + DB_RNTRY_SIZE(ans_section);
        rply_packet = malloc(rply_length );
        *(unsigned short*)rply_packet = rply_length;
        init_dns_header((struct dns_header_t*)((unsigned char *)rply_packet + 2),dns_h->id,error_code,0,0,1,0);
        init_rr_section(rply_packet + 2 + DNS_HEADER_SIZE,ans_section);

    }
    else if(add_section != NULL && ans_section != NULL){
        rply_length = 2 + DNS_HEADER_SIZE + DB_RNTRY_SIZE(ans_section) + DB_RNTRY_SIZE(add_section);
        rply_packet = malloc(rply_length);
        //memset(rply_packet,0,rply_length);
        *(unsigned short*)rply_packet = rply_length;
        init_dns_header((struct dns_header_t*)((unsigned char *)rply_packet + 2),dns_h->id,error_code,0,0,1,1);
        init_rr_section(rply_packet + 2 + DNS_HEADER_SIZE,ans_section);
        struct db_entry_t* check = get_rr_entry(rply_packet+2+DNS_HEADER_SIZE);
        //printf("1 domain name: %s data: %s\n",check->domain_name,check->data);
        init_rr_section(rply_packet + 2 + DNS_HEADER_SIZE + DB_RNTRY_SIZE(ans_section),add_section);
        //check = get_rr_entry(rply_packet+2+DNS_HEADER_SIZE);
        //printf("2 domain name: %s data: %s\n",check->domain_name,check->data);
        //check = get_rr_entry(rply_packet+2+DNS_HEADER_SIZE+DB_RNTRY_SIZE(ans_section));
        //printf("3 domain name: %s data: %s\n",check->domain_name,check->data);
        //printf("length: %d\n",rply_length);
    }
    else{
        rply_length = 2 + DNS_HEADER_SIZE;
         rply_packet = malloc(rply_length);
        *(unsigned short*)rply_packet = rply_length;
        init_dns_header((struct dns_header_t*)((unsigned char *)rply_packet + 2),dns_h->id,error_code,0,0,0,0);

    }
    if (send(socketfd, rply_packet, rply_length, 0) < 0) {
            printf("Send failed");
    }
    free(rply_packet);
}
#endif // DNS
