#include<stdio.h>
#include<stdlib.h>
#include"dns_db.h"
#include"dns_protocal.h"
#include"list.h"

char cache[] = "cache.txt";
struct list_head cache_head;

int main(){
    unsigned char buf[100];
    unsigned char name1[] = "中科院.中国";
    unsigned char data1[] = "10.0.2.15";
    unsigned char name2[] = "计算所.中科院.中国";
    unsigned char data2[] = "可猫.何胖胖";
    struct db_entry_t *rr1, *rr2;
    struct cache_entry_t c_entry1, c_entry2;
    init_list_head(&cache_head);
    init_list_head(&db_head);
    struct db_t db;
    c_entry1.rr.domain_name = name1;
    c_entry2.rr.domain_name = name2;
    c_entry1.rr.type = 1;
    c_entry1.rr._class = 1;
    c_entry1.rr.ttl = 1000;
    c_entry1.rr.length = 9;
    c_entry1.rr.data = data1;
    c_entry2.rr.type = 1;
    c_entry2.rr._class = 1;
    c_entry2.rr.ttl = 1000;
    c_entry2.rr.length = 16;
    c_entry2.rr.data = data2;

    insert_rr(&c_entry2,&cache_head,cache);
    insert_rr(&c_entry1,&cache_head,cache);
    
    db.name = cache;
    db.hd = fopen(cache,"r");
    list_add_tail(&db.list, &db_head);
    
    rr1 = find_rr_in_file(1,name2);

    printf("%s\n",rr1->data);

    scanf("%s",buf);

    rr2 = find_rr_in_file(1,buf);

    if(rr2 != NULL) printf("%s\n",rr2->data);
    else
    {
        printf("Can not find!\n");
    }

    return 0;
}
