#ifndef DNS_DB
#define DNS_DB
#include "list.h"

struct db_t{
    struct list_head list;
    unsigned int name_len;
    char *name;
    FILE *hd;
};

struct list_head db_head;

db_entry *find_rr_in_file(unsigned shor type, unsigned char* domain_name){

}

int insert_rr(){

}

int delete_rr(){

}

#endif // DNS_DB
