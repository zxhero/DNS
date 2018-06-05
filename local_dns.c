#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "dns_protocal.h"
#include "dns_db.h"

const char  cache[] = "cache.txt";
int socketfd,csocket;

void *checking_cache_thread(void *param){
    while(1){

    }
}

void    handle_packet(unsigned char *packet, int length){
    dns_header *dns_h = (struct dns_header_t*)(packet + 2);
    if(QR_query == ntohs(dns_h->tag))   handle_dns_query(packet+2, length);
    else if(QR_response & ntohs(dns_h->tag) != 0){

    }
    else{
        printf("receive wrong packet\n");
        return -1;
    }
}

void    handle_dns_query(unsigned char *packet, int length){
    struct dns_query_t* dns_q = get_ques_section(packet);
    if(dns_q->qtype != MX_ && dns_q->qtype != CNAME_ && dns_q->qtype != A_){
        reply_dns_query(packet, NULL, NULL, Type_Not_Support);
    }
    db_entry *ans_section = find_rr_in_file(dns_q->qtype, dns_q->dormain_name);
    if(ans_section == NULL){
        printf("do not find, send packet to remote dns\n");
    }
    else{
        if(dns_q->qtype == MX_){
            db_entry *add_section = find_rr_in_file(A_, ans_section->data);
            reply_dns_query(packet, ans_section, add_section, NO_ERROR);
            free(add_section->data);
            free(add_section->domain_name);
            free(add_section);
        }
        else if(dns_q->qtype == CNAME_ || dns_q->qtype == A_){
            reply_dns_query(packet, ans_section, NULL, NO_ERROR);
        }
    }
    free(dns_q);
    free(ans_section->data);
    free(ans_section->domain_name);
    free(ans_section);
}

void    reply_dns_query(unsigned char *packet, db_entry *ans_section, db_entry *add_section, int error_code){
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
        *(unsigned short*)rply_packet = rply_length;
        init_dns_header((struct dns_header_t*)((unsigned char *)rply_packet + 2),dns_h->id,error_code,0,0,1,1);
        init_rr_section(rply_packet + 2 + DNS_HEADER_SIZE,ans_section);
        init_rr_section(rply_packet + 2 + DNS_HEADER_SIZE + DB_RNTRY_SIZE(ans_section),add_section);

    }
    else{
        rply_length = 2 + DNS_HEADER_SIZE;
         rply_packet = malloc(rply_length);
        *(unsigned short*)rply_packet = rply_length;
        init_dns_header((struct dns_header_t*)((unsigned char *)rply_packet + 2),dns_h->id,error_code,0,0,0,0);

    }
    if (send(csocket, rply_packet, rply_length, 0) < 0) {
            printf("Send failed");
            return 1;
    }
    free(rply_packet);
}

void    send_dns_query(){

}

int main(){
    //int ;
    struct sockaddr_in server, client;

    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Could not create socket");
		return -1;
    }

    server.sin_family = AF_INET;
    //server.sin_addr.s_addr = INADDR_ANY;
    server.sin_addr.s_addr = inet_addr("127.1.1.1");
    server.sin_port = htons(53);

    if (bind(socketfd,(struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed. Error");
        return -1;
    }

    listen(socketfd, 1);

    int len = sizeof(struct sockaddr_in);
    if ((csocket = accept(socketfd, (struct sockaddr *)&client, (socklen_t *)&len)) < 0) {
        perror("accept failed");
        return 1;
    }

    int msg_len;
    unsigned char *packet = malloc(1024);
    while(1){
        memset(packet,0,1024);
        msg_len = recv(csocket, packet, 1024, 0);
        if (msg_len < 0) {
            perror("recv failed");
            return -1;
        }
        handle_packet(packet,msg_len);
    }
    return 0;
}
