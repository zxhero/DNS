#ifndef DNS_DB
#define DNS_DB
#include "list.h"

typedef struct db_entry_t{
    unsigned char*  domain_name;
    unsigned short  type;
    unsigned short  _class;
    unsigned int  ttl;
    unsigned short  length;
    unsigned char*  data;
}db_entry;

struct db_t{
    struct list_head list;
    unsigned int name_len;
    char *name;
    FILE *hd;
};

struct list_head db_head;

db_entry *find_rr(){

}

int insert_rr(){

}

int delete_rr(){

}
#endif // DNS_DB
