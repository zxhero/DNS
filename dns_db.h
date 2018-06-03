#ifndef DNS_DB
#define DNS_DB
#include "list.h"
#include "dns_protocal.h"
#include<string.h>
#include<stdio.h>

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

db_entry *find_rr_in_file(unsigned short type, unsigned char* domain_name){   //domain_name is host domain name
    struct db_t *db_file = NULL;
    //int is_exist = 0;
    //unsigned char *host_domain_name = malloc(100);
    //memset(host_domain_name,0,200);
    unsigned int max_field_num = 0;
    db_entry *rr = malloc(sizeof(db_entry));
    memset(rr,0,sizeof(db_entry));
    unsigned int h_type = 0;
    unsigned char   host_domain_name[100];
    memset(host_domain_name,0,100);

    list_for_each_entry(db_file,&db_head,list){
        fseek(db_file->hd,0,SEEK_SET);
        while(fscanf(db_file->hd,"%s %d",host_domain_name,&h_type) != EOF){
            ///printf("%s\n", host_domain_name);
            unsigned int compare_feild_num = 0;
            unsigned char* name_field = get_last_field_name(domain_name);
            unsigned char* h_name_field = get_last_field_name(host_domain_name);
            unsigned char* nptr;
            unsigned char* hptr;
            int is_wrong = 0;
            while(name_field != NULL && h_name_field != NULL){
                nptr = name_field;
                hptr = h_name_field;
                while(*nptr != '\0' && *nptr != '.' && *hptr != '\0' && *hptr != '.'){
                    if(*nptr++ != *hptr++){
                        is_wrong = 1;
                        break;
                    }
                }
                if(!is_wrong){
                    name_field = get_prior_field_name(name_field,domain_name);
                    h_name_field = get_prior_field_name(h_name_field,host_domain_name);
                    compare_feild_num ++;
                }
                else    break;
            }
            //printf("is_wrong: %d\n",is_wrong);
            if(is_wrong || (name_field == NULL && h_name_field != NULL))    fgets(host_domain_name,100,db_file->hd);
            else if(h_type == type && compare_feild_num > max_field_num){
                //printf("is_wrong: %d\n",is_wrong);
                if(rr->domain_name != NULL) free(rr->domain_name);
                unsigned int length = get_domain_name_len(host_domain_name);
                //printf("is_wrong: %d\n",is_wrong);
                rr->domain_name = malloc(length);
                memcpy(rr->domain_name,host_domain_name,length);
                rr->type = h_type;
                fscanf(db_file->hd,"%d %d %d",&rr->_class,&rr->ttl,&rr->length);
                if(rr->data != NULL)    free(rr->data);
                rr->data = malloc(rr->length + 1);
                fscanf(db_file->hd,"%s",rr->data);
                max_field_num = compare_feild_num;
                //printf("%s\n",rr->data);
            }
            else{
                fgets(host_domain_name,100,db_file->hd);
            }
            memset(host_domain_name,0,100);
            h_type = 0;
        }
    }
    //printf("max_field_num: %d\n",max_field_num);
    if(max_field_num != 0)  return rr;
    else{
        free(rr);
        return NULL;
    }
}

void    print_cache(FILE *fd, struct list_head *cache_head){
    struct cache_entry_t* cache_entry = NULL;
    if(!list_empty(cache_head)){
        list_for_each_entry(cache_entry,cache_head,list){
            fprintf(fd,"%s %d %d %d %d %s\n",cache_entry->rr.domain_name,cache_entry->rr.type,cache_entry->rr._class,cache_entry->rr.ttl,
                    cache_entry->rr.length,cache_entry->rr.data);
        }
    }
}

int insert_rr(struct cache_entry_t* cache_entry, struct list_head *cache_head, char *cache_file){
    FILE* fd;
    fd = fopen(cache_file,"w");
    list_add_tail(&cache_entry->list, cache_head);
    print_cache(fd, cache_head);
    fclose(fd);
    return 1;
}

int delete_rr(struct cache_entry_t* cache_entry, struct list_head *cache_head, char *cache_file){
    FILE* fd;
    fd = fopen(cache_file,"w");
    list_delete_entry(&cache_entry->list);
    print_cache(fd, cache_head);
    fclose(fd);
    return 1;
}

unsigned char* ntoh_domain_name(unsigned char* n_domain_name){
    unsigned int length = 0;
    unsigned char buff[100], *dest = buff;
    unsigned char* name = n_domain_name;
    //unsigned char* name_next = n_domain_name;
    unsigned char len = 0;
    while(*name != '\0'){
        len = *name++;
        length += len;
        for(int i = 0;i < len;i++){
            *dest++ = *name++;
        }
        if(*name == '\0')   *dest = '\0';
        else    *dest++ = *name++;
        length++;
    }
    unsigned char* h_domain_name = malloc(length);
    memcpy(h_domain_name, buff, length);
    return h_domain_name;
}

#endif // DNS_DB
