#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include "dns_protocal.h"
#include "dns_db.h"


void    init_db(char *sever_name[], int sever_num){
    init_list_head(&db_head);
    for(int i = 0; i < sever_num;i++){
        struct db_t db_file = malloc(sizeof(struct db_t));
        db_file->name = sever_name[i];
        db_file->hd = fopen(db_file->name,"r");
        list_add_tail(&db_file->list, &db_head);
    }
}

void    handle_packet(){

}

void    handle_dns_query(){

}

void    reply_dns_query(){

}

int main(int argc, char *argv[]){

}
