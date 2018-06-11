#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include<unistd.h>
#include "dns_protocal.h"
#include "dns_db.h"
#include "dns.h"

int socketfd,csocket;

void    init_db(char *sever_name[], int sever_num){
    printf("%s\n",sever_name[0]);
    init_list_head(&db_head);
    for(int i = 0; i < sever_num;i++){
        struct db_t *db_file = malloc(sizeof(struct db_t));
        db_file->name = sever_name[i];
        db_file->hd = fopen(db_file->name,"r");
        list_add_tail(&db_file->list, &db_head);
    }
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

    }
    else{
        printf("Find Data: %s\n",ans_section->data);
        if(compare_domain_name(ans_section->domain_name,dns_q->dormain_name) == 1){
            if(dns_q->qtype == MX_){
                db_entry *add_section = find_rr_in_file(A_, ans_section->data);
                printf("Add name: %s Add Data: %s\n",add_section->domain_name,add_section->data);
                reply_dns_query(packet, ans_section, add_section, NO_ERROR,csocket);
                free(add_section->data);
                free(add_section->domain_name);
                free(add_section);
            }
            else if(dns_q->qtype == CNAME_ || dns_q->qtype == A_){
                reply_dns_query(packet, ans_section, NULL, NO_ERROR,csocket);
            }
        }
        else{
            reply_dns_query(packet, ans_section, NULL, NO_ERROR,csocket);
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

int main(int argc, char *argv[]){
    init_db(argv+1,argc-1);

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
    server.sin_addr.s_addr = inet_addr("127.5.2.1");
    server.sin_port = htons(53);

    if (bind(socketfd,(struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed. Error");
        return -1;
    }

    listen(socketfd, 3);

    int len = sizeof(struct sockaddr_in);

    int msg_len;
    unsigned char *packet = malloc(1024);
    while(1){
        if ((csocket = accept(socketfd, (struct sockaddr *)&client, (socklen_t *)&len)) < 0) {
            perror("accept failed");
            return 1;
        }
        memset(packet,0,1024);
        //printf("2\n");
        msg_len = recv(csocket, packet, 1024, 0);
        if (msg_len < 0) {
            perror("recv failed");
            return -1;
        }
        //printf("length: %d\n",msg_len);
        if(msg_len != 0)    handle_packet(packet,msg_len);
        close(csocket);
    }
    return 0;
}
