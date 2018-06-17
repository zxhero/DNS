#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include<unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "dns_protocal.h"
#include "dns_db.h"
#include "dns.h"

char  cache[] = "cache.txt";
int socketfd,csocket,rsocket;
struct sockaddr_in root_dns;
struct list_head cache_head;
pthread_mutex_t cache_lock;

void *checking_cache_thread(void *param){
    while(1){
        sleep(FLUSH_CACHE_TIME);
        //printf("FLUSH!\n");
        if(!list_empty(&cache_head)){
            struct cache_entry_t *cache_entry = NULL;
            struct cache_entry_t *cache_next = NULL;
            unsigned int tic = time(NULL);
            list_for_each_entry_safe(cache_entry,cache_next,&cache_head,list){
                if(tic - cache_entry->alive > cache_entry->rr->ttl){
                    //printf("FLUSH one \n");
                    pthread_mutex_lock(&cache_lock);
                    delete_rr(cache_entry,&cache_head,cache);
                    pthread_mutex_unlock(&cache_lock);
                }
            }
        }
    }
}

void    send_dns_query(unsigned char* packet, unsigned int length, unsigned char* domain_name){
        struct sockaddr_in remote_dns = root_dns;
        struct sockaddr_in src_dns;
        socklen_t src_len = sizeof(struct sockaddr_in);
        unsigned char* reply = malloc(512);
        unsigned char* local_reply = malloc(512);
        while(1){
            if ((rsocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("Could not create socket");
            }
            /*if ((connect(rsocket, (struct sockaddr *)&remote_dns, sizeof(remote_dns)) < 0)) {
                perror("connect failed. Error");
            }*/
            if (sendto(rsocket, packet, length, 0,(struct sockaddr *)&remote_dns, sizeof(remote_dns)) < 0) {
                printf("Send failed");
            }
            memset(reply,0,512);
            unsigned int reply_len = recvfrom(rsocket, reply, 512, 0, (struct sockaddr *)&src_dns, &src_len);
            if (reply_len < 0) {
                perror("recv failed");
            }
            dns_header * dns_h = (dns_header *)(reply);
            //printf("id: %d\n",ntohs(dns_h->id));
            memset(local_reply,0,512);
            memcpy(local_reply+2,reply,reply_len);
            *(unsigned short*)local_reply = reply_len + 2;
            if(dns_h->tag & Name_Not_Exist != 0){
                send(csocket, local_reply, reply_len+2, 0);
            }
            else if(dns_h->tag & Type_Not_Support != 0){
                send(csocket, local_reply, reply_len+2, 0);
            }
            else{
                db_entry *ans_section = get_rr_entry(reply+DNS_HEADER_SIZE);
                if(compare_domain_name(domain_name,ans_section->domain_name) == 1){
                    struct dns_query_t* dns_q = get_ques_section(packet);
                    if(dns_q->qtype == MX_){
                        db_entry *add_section = get_rr_entry(reply+DNS_HEADER_SIZE+(10 + get_domain_name_len(reply+DNS_HEADER_SIZE) + ans_section->length + 1));
                        struct cache_entry_t *cache_entry_ans = malloc(sizeof(struct cache_entry_t));
                        struct cache_entry_t *cache_entry_add = malloc(sizeof(struct cache_entry_t));
                        cache_entry_ans->rr = ans_section;
                        cache_entry_ans->alive = time(NULL);
                        cache_entry_add->rr = add_section;
                        cache_entry_add->alive = time(NULL);
                        pthread_mutex_lock(&cache_lock);
                        insert_rr(cache_entry_ans,&cache_head,cache);
                        insert_rr(cache_entry_add,&cache_head,cache);
                        pthread_mutex_unlock(&cache_lock);
                    }
                    else{
                        struct cache_entry_t *cache_entry = malloc(sizeof(struct cache_entry_t));
                        cache_entry->rr = ans_section;
                        cache_entry->alive = time(NULL);
                        pthread_mutex_lock(&cache_lock);
                        insert_rr(cache_entry,&cache_head,cache);
                        pthread_mutex_unlock(&cache_lock);
                    }
                    printf("Send DNS response to Client \n");
                    send(csocket, local_reply, reply_len+2, 0);
                    free(dns_q->dormain_name);
                    free(dns_q);
                    break;
                }
                else{
                    close(rsocket);
                    remote_dns.sin_addr.s_addr = inet_addr(ans_section->data);
                    printf("Send DNS query to DNS: %s\n",ans_section->data);
                    //remote_dns.sin_addr.s_addr = ntohl(*(ans_section->data));
                    free(ans_section->domain_name);
                    free(ans_section->data);
                    free(ans_section);
                }
            }
        }
        free(reply);
        free(local_reply);
}

void    handle_dns_query(unsigned char *packet, int length){
    struct dns_query_t* dns_q = get_ques_section(packet);
    unsigned char* reply_packet = NULL;
    int reply_len = 0;
    if(dns_q->qtype != MX_ && dns_q->qtype != CNAME_ && dns_q->qtype != A_){
        reply_len = reply_dns_query(packet, NULL, NULL, Type_Not_Support,&reply_packet);
        send(csocket, reply_packet, reply_len, 0);
        free(reply_packet);
    }
    printf("Receive : %s %d\n",dns_q->dormain_name, dns_q->qtype);

    pthread_mutex_lock(&cache_lock);
    db_entry *ans_section = find_rr_in_file(dns_q->qtype, dns_q->dormain_name);
    pthread_mutex_unlock(&cache_lock);
    if(ans_section == NULL){
        printf("do not find, send packet to root dns\n");
        send_dns_query(packet,length-2,dns_q->dormain_name);
    }
    else{
        printf("Find Data: %s\n",ans_section->data);
        if(compare_domain_name(ans_section->domain_name,dns_q->dormain_name) != 1){
            send_dns_query(packet,length-2,dns_q->dormain_name);
        }
        else{
            if(dns_q->qtype == MX_){
                pthread_mutex_lock(&cache_lock);
                db_entry *add_section = find_rr_in_file(A_, ans_section->data);
                pthread_mutex_unlock(&cache_lock);
                reply_len = reply_dns_query(packet, ans_section, add_section, NO_ERROR,&reply_packet);
                free(add_section->data);
                free(add_section->domain_name);
                free(add_section);
            }
            else if(dns_q->qtype == CNAME_ || dns_q->qtype == A_){
                reply_len = reply_dns_query(packet, ans_section, NULL, NO_ERROR,&reply_packet);
            }
            send(csocket, reply_packet, reply_len, 0);
            free(reply_packet);
        }
        free(ans_section->data);
        free(ans_section->domain_name);
        free(ans_section);
    }
    free(dns_q->dormain_name);
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
 void init(){
    struct db_t *db = malloc(sizeof(struct db_t));
    db->name = cache;
    db->hd = NULL;
    init_list_head(&db_head);
    list_add_tail(&db->list, &db_head);
    init_list_head(&cache_head);
    pthread_mutex_init(&cache_lock,NULL);
    pthread_t cache_flush;
    pthread_create(&cache_flush, NULL, checking_cache_thread, NULL);
 }

int main(){
    //int ;
    init();

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
