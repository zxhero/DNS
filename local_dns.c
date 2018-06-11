#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include<unistd.h>
#include <arpa/inet.h>
#include "dns_protocal.h"
#include "dns_db.h"
#include "dns.h"

char  cache[] = "cache.txt";
int socketfd,csocket,rsocket;
struct sockaddr_in root_dns;
char *current_domainame;

void *checking_cache_thread(void *param){
    while(1){

    }
}

void    send_dns_query(unsigned char* packet, unsigned int length, unsigned char* domain_name){
        struct sockaddr_in remote_dns = root_dns;
        unsigned char* reply = malloc(512);
        while(1){
            if ((rsocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Could not create socket");
            }
            if ((connect(rsocket, (struct sockaddr *)&remote_dns, sizeof(remote_dns)) < 0)) {
                perror("connect failed. Error");
            }
            if (send(rsocket, packet, length, 0) < 0) {
                printf("Send failed");
            }
            memset(reply,0,512);
            unsigned int reply_len = recv(rsocket, reply, 512, 0);
            if (reply_len < 0) {
                perror("recv failed");
            }
            dns_header * dns_h = (dns_header *)(reply + 2);
            if(dns_h->tag & Name_Not_Exist != 0){
                send(csocket, reply, reply_len, 0);
            }
            else if(dns_h->tag & Type_Not_Support != 0){

                send(csocket, reply, reply_len, 0);
            }
            else{
                db_entry *ans_section = get_rr_entry(reply+2+DNS_HEADER_SIZE);
                if(compare_domain_name(domain_name,ans_section->domain_name) == 1){
                    send(csocket, reply, reply_len, 0);
                    break;
                }
                else{
                    close(rsocket);
                    remote_dns.sin_addr.s_addr = inet_addr(ans_section->data);
                }
            }
        }
        free(reply);
}

void    handle_dns_query(unsigned char *packet, int length){
    struct dns_query_t* dns_q = get_ques_section(packet);
    if(dns_q->qtype != MX_ && dns_q->qtype != CNAME_ && dns_q->qtype != A_){
        reply_dns_query(packet, NULL, NULL, Type_Not_Support,csocket);
    }
    printf("Receive : %s %d\n",dns_q->dormain_name, dns_q->qtype);

    db_entry *ans_section = find_rr_in_file(dns_q->qtype, dns_q->dormain_name);
    if(ans_section == NULL){
        printf("do not find, send packet to remote dns\n");
        send_dns_query(packet - 2,length,dns_q->dormain_name);
    }
    else{
        printf("Find Data: %s\n",ans_section->data);
        if(compare_domain_name(ans_section->domain_name,dns_q->dormain_name) != 1){
            send_dns_query(packet - 2,length,dns_q->dormain_name);
        }
        else{
            if(dns_q->qtype == MX_){
                db_entry *add_section = find_rr_in_file(A_, ans_section->data);
                reply_dns_query(packet, ans_section, add_section, NO_ERROR,csocket);
                free(add_section->data);
                free(add_section->domain_name);
                free(add_section);
            }
            else if(dns_q->qtype == CNAME_ || dns_q->qtype == A_){
                reply_dns_query(packet, ans_section, NULL, NO_ERROR,csocket);
            }
        }
        free(ans_section->data);
        free(ans_section->domain_name);
        free(ans_section);
    }
    free(dns_q);
}

void    handle_packet(unsigned char *packet, int length){
    dns_header *dns_h = (struct dns_header_t*)(packet + 2);
    if(QR_query == ntohs(dns_h->tag))   {
            handle_dns_query(packet+2, length);
    }
    else{
        printf("receive wrong packet\n");
    }
}

int main(){
    //int ;
    struct db_t db;
    db.name = cache;
    db.hd = fopen(cache,"r");
    init_list_head(&db_head);
    list_add_tail(&db.list, &db_head);

    struct sockaddr_in server, client;

    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Could not create socket");
		return -1;
    }

    int val = 1;
     int ret = setsockopt(socketfd,SOL_SOCKET,SO_REUSEADDR,(void *)&val,sizeof(int));
     if(ret == -1)
     {
         printf("setsockopt");
         exit(1);
     }

    server.sin_family = AF_INET;
    //server.sin_addr.s_addr = INADDR_ANY;
    server.sin_addr.s_addr = inet_addr("127.3.2.1");
    server.sin_port = htons(53);
    root_dns.sin_family = AF_INET;
    //server.sin_addr.s_addr = INADDR_ANY;
    root_dns.sin_addr.s_addr = inet_addr("127.5.2.1");
    root_dns.sin_port = htons(53);
    if (bind(socketfd,(struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed. Error");
        return -1;
    }

    listen(socketfd, 3);

    int len = sizeof(struct sockaddr_in);
    if ((csocket = accept(socketfd, (struct sockaddr *)&client, (socklen_t *)&len)) < 0) {
        perror("accept failed");
        return 1;
    }

    int msg_len;
    unsigned char *packet = malloc(1024);
    while(1){
        memset(packet,0,1024);
        //printf("2\n");
        msg_len = recv(csocket, packet, 1024, 0);
        if (msg_len < 0) {
            perror("recv failed");
            return -1;
        }
        //printf("length: %d\n",msg_len);
        if(msg_len != 0)    handle_packet(packet,msg_len);
    }
    return 0;
}
