#include<stdio.h>
#include "dns_protocal.h"
#include <sys/socket.h>

int socketfd = socket(AF_INET,SOCK_STREAM,0);

void handle_input(){
    while(scanf("") != EOF){

    }
}

void send_dns_packet(){

}

void handle_dns_operate(int qtype, int qclass, char* domain_name){
    char *packet
}

int main(){
    struct sockaddr_in client;

    client.sin_addr.s_addr = inet_addr("127.1.1.1");
    client.sin_family = AF_INET;
    client.sin_port = htons( 53 );

    if ((connect(socketfd, (struct sockaddr *)&client, sizeof(client)) < 0)) {
        perror("connect failed. Error");
        return 1;
    }

    handle_input();
    return 0;
}
