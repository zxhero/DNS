#include<stdio.h>
#include "dns_protocal.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include<unistd.h>
int socketfd;

unsigned char* send_dns_packet(unsigned char* packet,unsigned int length,unsigned char *domain_name){
    unsigned char* reply = malloc(512);

    //int valid = 0;
    int send_socket = socketfd;
    struct sockaddr_in remote_dns;
    remote_dns.sin_family = AF_INET;
    remote_dns.sin_port = htons( 53 );
    //while(valid == 0){
        if (send(send_socket, packet, length, 0) < 0) {
            printf("Send failed");
           // return 1;
        }
        memset(reply,0,512);
        unsigned int reply_len = recv(send_socket, reply, 512, 0);
        //printf("length: %d\n",reply_len);
        if (reply_len < 0) {
            perror("recv failed");
           // return -1;
        }
        dns_header * dns_h = (dns_header *)(reply + 2);
        if(dns_h->tag & Name_Not_Exist != 0){
            printf("Name_Not_Exist\n");
            return NULL;
        }
        else if(dns_h->tag & Type_Not_Support != 0){
            printf("Type_Not_Support\n");
            return NULL;
        }
        struct db_entry_t* ans_section = get_rr_entry(reply+2+DNS_HEADER_SIZE);
        //printf("domain name: %s data: %s\n",ans_section->domain_name,ans_section->data);
        //printf("domain name: %s\n",domain_name);
        if(compare_domain_name(domain_name,ans_section->domain_name) != 1){
            printf("wrong!\n");
            send_socket = socket(AF_INET,SOCK_STREAM,0);

            remote_dns.sin_addr.s_addr = inet_addr(ans_section->data);


            if ((connect(send_socket, (struct sockaddr *)&remote_dns, sizeof(remote_dns)) < 0)) {
                perror("connect failed. Error");
               // return 1;
            }
        }
        else{
            printf("%s %d %s\n",ans_section->domain_name,ans_section->type,ans_section->data);
            if(dns_h->add_count != 0)  {
                struct db_entry_t* add_section = get_rr_entry(reply+2+DNS_HEADER_SIZE+(10 + get_domain_name_len(reply+2+DNS_HEADER_SIZE) + ans_section->length + 1));
                printf("%s %d %s\n",add_section->domain_name,add_section->type,add_section->data);
                free(add_section->data);
                free(add_section->domain_name);
                free(add_section);
            }
            //valid = 1;
        }
        free(ans_section->data);
        free(ans_section->domain_name);
        free(ans_section);
    //}
    return reply;
}

void handle_dns_operate(int qtype, int qclass, char* domain_name){

    dns_query dns_q;
    unsigned char buff[100];
    memset(buff,0,100);
    unsigned int length = hton_domain_name(buff,domain_name);
    dns_q.dormain_name = malloc(length);
    memset(dns_q.dormain_name,0,length);
    memcpy(dns_q.dormain_name,buff,length);
    dns_q.qtype = qtype;
    dns_q.qclass = qclass;
    unsigned short packet_len = 2 + DNS_HEADER_SIZE + length + 4;
    unsigned char *packet = malloc(packet_len);
    *(unsigned short *)packet = packet_len;
    init_dns_header((dns_header*)(packet+2),1,0,1,1,0,0);
    init_ques_section((unsigned char *)packet+2,dns_q.dormain_name,length,dns_q.qtype,dns_q.qclass);
    //printf("domain name: %s\n",domain_name);
    unsigned char* reply = send_dns_packet(packet,packet_len, domain_name);
    free(packet);
    free(reply);
    free(dns_q.dormain_name);
}

void handle_input(){
    unsigned char domain_name[100], type[6];
    //unsigned char packet[1024];
    unsigned int reply_len= 0;
    memset(domain_name,0,100);
    memset(type,0,6);
    while(scanf("%s %s",domain_name,type) != EOF){
        if(domain_name[0] == '0'){
            close(socketfd);
            break;
        }
        //printf("1\n");
        //printf("Type : %s\n",type);
        if(type[0] == 'M' ){
            handle_dns_operate(MX_, IN_, domain_name);
        }
        else if(type[0] == 'A'){
            handle_dns_operate(A_, IN_, domain_name);
        }
        else if(type[0] == 'C'){
            handle_dns_operate(CNAME_, IN_, domain_name);
        }
        else{
            printf("Wrong Type : %s\n",type);
          //  return 1;
        }
        memset(domain_name,0,100);
        memset(type,0,6);
        //reply_len = recv(socketfd, packet, 1024, 0);
    }
}

int main(){
    struct sockaddr_in client;

    socketfd = socket(AF_INET,SOCK_STREAM,0);

    client.sin_addr.s_addr = inet_addr("127.3.2.1");
    client.sin_family = AF_INET;
    client.sin_port = htons( 53 );

    if ((connect(socketfd, (struct sockaddr *)&client, sizeof(client)) < 0)) {
        perror("connect failed. Error");
        return 1;
    }

    handle_input();
    return 0;
}
