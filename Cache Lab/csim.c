#include "cachelab.h"
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct cache_line{
    unsigned int valid;
    unsigned int tag;
    unsigned int time;
}Cache_line;

typedef struct cache{
    unsigned int S;
    unsigned int E;
    unsigned int B;
    Cache_line **lines;
}Cache;

unsigned int hits=0, misses=0, evictions=0, nums=0;
unsigned int s, E, b;
bool v = false;
char *f = NULL;
Cache *cache = NULL;

void print_help()
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("-h         Print this help message.\n");
    printf("-v         Optional verbose flag.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n\n\n");
    printf("Examples:\n");
    printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void init_Cache(unsigned int s, unsigned int E, unsigned int b){
    unsigned int S = 1 << s;
    unsigned int B = 1 << b;
    cache = (Cache *)malloc(sizeof(Cache)); //分配存放结构Cache内存
    cache->B = B;
    cache->E = E;
    cache->S = S;
    cache->lines = (Cache_line **)malloc(sizeof(Cache_line *) * S); //分配S个存放Cache_line数组地址的内存
    for(int i=0; i<S; i++){
        cache->lines[i] = (Cache_line *)malloc(sizeof(Cache_line) * E); //分配E个存放结构Cache_line的内存，即有S个Cache_line数组，每个数组都有E个Cache_line结构。
        for(int j=0; j<E; j++){
            cache->lines[i][j].tag = 0;
            cache->lines[i][j].valid = 0;
            cache->lines[i][j].time = 0;
        }
    }
}

void free_Cache(){  //逐步释放分配的内存
    unsigned int S = cache->S;
    for(int i=0; i<S; i++)  free(cache->lines[i]);  
    free(cache->lines);
    free(cache);
}

unsigned int find_oldeat(unsigned int set){
    Cache_line *lines = cache->lines[set];
    int data=__INT32_MAX__, site=0;
    for(int i=0; i<E; i++){
        if(lines[i].time < data){
            data = lines[i].time;
            site = i;
        }
    }
    return site;
}

void load_line(char flag, unsigned int addr, unsigned int size){
    unsigned int tag = addr >> (b+s);
    unsigned int set = (((1 << (b+s)) - 1) & addr)>> b;
    Cache_line *lines = cache->lines[set];
    bool find = false, full = true;
    int site;

    if(v)   printf("%c %x,%x", flag, addr, size);

    for(int i=0; i<E; i++){
        if(lines[i].valid == 1){
            if(lines[i].tag == tag){
                lines[i].time = nums;
                hits++;
                if(v)   printf(" hit");
                find = true;
                break;
            }
        }
        else{
            full = false;
            site = i;
        }
    }

    if(!find){
        misses++;
        if(v)   printf(" miss");
        if(!full){
            lines[site].valid = 1;
        }
        else{
            evictions++;
            if(v)   printf(" evictions");
            site = find_oldeat(set);
        }
        lines[site].tag = tag;
        lines[site].time = nums;
    }

    if(flag == 'M'){
        hits++;
        if(v)   printf(" hit");
    }
    printf("\n");
    
}

void run_cache(FILE *file){
    char flag;
    unsigned int addr, size;
    while(fscanf(file, " %c %x,%x", &flag, &addr, &size) > 0){
        if(flag == 'I') continue;
        load_line(flag, addr, size);
        nums++;
    }
}

int main(int argc, char *argv[])
{
    int opt;
    while((opt = getopt(argc, argv, "h:v:s:E:b:t:")) != -1){
        switch(opt){
            case 'h':
                print_help();
                exit(0);
            case 'v':
                v = true;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                f = optarg;
                break;
            default:
                printf("./csim-ref: invalid option -- '%s'", argv[0]);
                print_help();
                return 1;
        }
    }

    init_Cache(s, E, b);
    FILE *file = fopen(f, "r");
    run_cache(file);
    free_Cache();

    printSummary(hits, misses, evictions);
    return 0;
}