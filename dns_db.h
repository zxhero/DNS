#ifndef DNS_DB
#define DNS_DB
#include "list.h"
#include "dns_protocal.h"

struct db_t{
    struct list_head list;
    //unsigned int name_len;
    char *name;
    FILE *hd;
};

struct cache_entry_t{
    struct list_head list;
    struct db_entry_t rr;
};

struct list_head db_head;
//struct list_head cache_head;

db_entry *find_rr_in_file(unsigned shor type, unsigned char* domain_name){
    struct db_t *db_file = NULL;
    //int is_exist = 0;
    unsigned char *host_domain_name = malloc(100);
    memset(host_domain_name,0,200);
    list_for_each_entry(db_file,&db_head,list){
        fseek(db_file->hd,0,SEEK_SET);
        unsigned int h_type = 0;
        while(fscanf("%s %d",host_domain_name,&h_type) != EOF){
            unsigned char* nptr = domain_name;
            unsigned char* hptr = host_domain_name;
            unsigned char bnum = *nptr++;
            int is_wrong = 0;
            while(*nptr != '\0'){
                for(int i = 0;i < bnum;i++){
                    if(*nptr==*hptr && *(nptr+1) == *(hptr+1) && *(nptr+2) == *(hptr+2)){
                        nptr += 3;
                        hptr += 3;
                    }
                    else{
                        is_wrong = 1;
                        break;
                    }
                }
                if(!is_wrong && *nptr != '\0'){
                    nptr++;
                    hptr++;
                }
                else    break;
            }
            if(is_wrong)    fgets()
            else if(h_type == type){
                db_entry *rr = malloc(sizeof(db_entry));
                rr->domain_name = host_domain_name;
                rr->type = h_type;
                fscanf("%d %d %d",&rr->_class,&rr->ttl,&rr->length);
                rr->data = malloc(rr->length + 1);
                fscanf("%s",rr->data);
                return rr;
            }
            memset(host_domain_name,0,200);
            h_type = 0;
        }
    }
}

void    print_cache(FILE *fd, struct list_head *cache_head){
    struct cache_entry_t* cache_entry = NULL;
    if(!list_empty(cache_head)){
        list_for_each_entry(cache_entry,cache_head,list){
            fprintf(fd,"%s %d %d %d %d %s\n",cache_entry->rr.domain_name,cache_entry->rr.type,cache_entry->rr._class,cache_entry->rr.ttl
                    cache_entry->rr.length,cache_entry->rr.data);
        }
    }
}

int insert_rr(struct cache_entry_t* cache_entry, struct list_head *cache_head, char *cache_file){
    FILE* fd;
    fd = fopen(cache_file,"w");
    list_add_tail(&cache_entry->list, cache_head);
    print_cache(fd, cache_head);
}

int delete_rr(struct cache_entry_t* cache_entry, struct list_head *cache_head, char *cache_file){
    FILE* fd;
    fd = fopen(cache_file,"w");
    list_delete_entry(&cache_entry->list);
    print_cache(fd, cache_head);
}

#endif // DNS_DB
