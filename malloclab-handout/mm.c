/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "sleep",
    /* First member's full name */
    "jxli",
    /* First member's email address */
    "1084548857@qq.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

#define wsize 4
#define dsize 8
#define maxsize 12
#define chunksize (1<<12)
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define pack(size, allco)   ((size) | (allco))
#define get(p) (*(unsigned int *)(p))
#define put(p, val) ((*(unsigned int *)(p)) = (val))
#define get_size(p) (get(p) & ~0x7)
#define get_alloc(p) (get(p) & 0x01)
#define hdrp(bp) ((char *)(bp) - wsize)
#define ftrp(bp) ((char *)(bp) + get_size(hdrp(bp)) - dsize)
#define next_blkp(bp) ((char *)(bp) + get_size(((char *)(bp) - wsize)))
#define prev_blkp(bp) ((char *)(bp) - get_size(((char *)(bp) - dsize)))
#define get_head(num) ((unsigned int *)(get(heap_listp + num*wsize)))
#define get_pre(bp) ((unsigned int *)get(bp))
#define get_suc(bp) ((unsigned int *)(get((unsigned int *)bp + 1)))

static char *heap_listp;

static int find_num(size_t size){
    int i;
    for(i=4; i<=maxsize; i++){
        if(size <= (1 << i))    return i-4;
    }
    return i-4;
}

static void print_list(){
    int num = 0;
    unsigned int *p;
    printf("print list \n");
    while(num <= 9){
        printf("num = %d \n", num);
        p = get_head(num);
        while(p != NULL){
            printf("p = %u \n", (unsigned int)p);
            p = get_suc(p);
        }
        num++;
    }
}

static void insert(void *bp){
    //printf("insert \n");

    size_t size = get_size(hdrp(bp));
    int num = find_num(size);
    if(get_head(num) == NULL){
        put(heap_listp+num*wsize, bp);
        put(bp, NULL);
        put((unsigned int *)bp+1, NULL);
    }
    else{
        put((unsigned int *)bp+1, get_head(num));
        put(get_head(num), bp);
        put(heap_listp+num*wsize, bp);
        put(bp, NULL);
    }

    //print_list();
    //printf("end list \n");
}


static void delate(void *bp){
    //printf("delate \n");

    size_t size = get_size(hdrp(bp));
    int num = find_num(size);
    unsigned int *pre = get_pre(bp);
    unsigned int *suc = get_suc(bp);
    unsigned int *head = get_head(num);

    if(pre == NULL && suc == NULL)  put(heap_listp+num*wsize, NULL);
    else if(pre == NULL && suc != NULL){
        //printf("2 \n");
        put(heap_listp+num*wsize, suc);
        put(suc, NULL);
    }
    else if(pre != NULL && suc == NULL){
        //printf("3 \n");
        put(pre+1, NULL);
    }
    else{
        //printf("4 \n");
        put(pre+1, suc);
        //printf("5 \n");
        put(suc, pre);
    }

    //print_list();
    //printf("end delate \n");
}

static void *coalesce(void *bp){
    //printf("coalesce\n");
    size_t prev = get_alloc(ftrp(prev_blkp(bp)));
    size_t next = get_alloc(hdrp(next_blkp(bp)));
    size_t size = get_size(hdrp(bp));

    if(prev && next){
        insert(bp);
        return bp;
    }
    else if(prev && !next){
        //printf("1 \n");

        delate(next_blkp(bp));
        size += get_size(hdrp(next_blkp(bp)));
        put(hdrp(bp), pack(size, 0));
        put(ftrp(bp), pack(size, 0));
        insert(bp);
    }
    else if(!prev && next){
        //printf("2 \n");

        delate(prev_blkp(bp));
        size += get_size(ftrp(prev_blkp(bp)));
        put(ftrp(bp), pack(size, 0));
        put(hdrp(prev_blkp(bp)), pack(size, 0));
        bp = prev_blkp(bp);

        //print_list(bp);        
        insert(bp);
    }
    else{
        delate(prev_blkp(bp));
        delate(next_blkp(bp));
        size += get_size(hdrp(next_blkp(bp))) + get_size(ftrp(prev_blkp(bp)));
        put(hdrp(prev_blkp(bp)), pack(size, 0));
        put(ftrp(next_blkp(bp)), pack(size, 0));
        bp = prev_blkp(bp);
        insert(bp);
    }

    //print_list();
    //printf("end coalesce \n");
    return bp;
}

//def extend_heap to extend the size of vertal memory
static void *extend_heap(size_t words){
    //printf("extend_heap:%u \n", words);

    char *bp;
    size_t size;
    size = (words % 2) ? (words+1) * wsize : (words) * wsize;
    //printf("size = %u\n", size);
    bp = mem_sbrk(size);

    //print_list();

    put(hdrp(bp), pack(size, 0));
    put(ftrp(bp), pack(size, 0));
    put(hdrp(next_blkp(bp)), pack(0, 1));

    //print_list();
    //printf("end extend \n");
    return coalesce(bp);
}

static void *find_fit(size_t asize){
    //printf("find \n");

    int num = find_num(asize);
    unsigned int* bp;;
    while(num <= (maxsize-3)){
        //printf("num = %d \n", num);
        bp = get_head(num);
        while(bp != NULL){
            //printf("4 \n");
            if(get_size(hdrp(bp)) >= asize){
                //printf("5 \n");
                //printf("bp = %u\n",(unsigned int)bp);
                return (void *)bp;
            }
            //printf("1 \n");
            bp = get_suc(bp);
            //printf("2 \n");
        }
        num++;
    }

    //print_list();
    //printf("end fit \n"); 
    return NULL;
}

static void place(void *bp, size_t asize){
    //printf("place \n");

    size_t csize = get_size(hdrp(bp));
    if((csize - asize) >= (2*dsize)){
        delate(bp);
        put(hdrp(bp), pack(asize, 1));
        put(ftrp(bp), pack(asize, 1));
        bp = next_blkp(bp);
        put(hdrp(bp), pack(csize-asize, 0));
        put(ftrp(bp), pack(csize-asize, 0));
        coalesce(bp);

        //printf("2 \n");
    }
    else{
        delate(bp);
        put(hdrp(bp), pack(csize, 1));
        put(ftrp(bp), pack(csize, 1));

        //printf("3 \n");
    }

    //print_list();
    
    //printf("end place \n");
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //printf("init \n");
    if((heap_listp = mem_sbrk((4+maxsize-2)*wsize)) == (void *)-1)
        return -1;
    for(int i=0; i<maxsize -2; i++){
        put(heap_listp+i*wsize, NULL);
    }
    heap_listp += (maxsize - 2)*wsize;
    put(heap_listp, 0);
    put(heap_listp + 1*wsize, pack(dsize, 1));
    put(heap_listp + 2*wsize, pack(dsize, 1));
    put(heap_listp + 3*wsize, pack(0, 1));
    heap_listp -= (maxsize - 2)*wsize;

    if(extend_heap(chunksize/wsize) == NULL)
        return -1;
    
    //printf("init \n");
    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    //printf("\n malloc:%u \n", size);
    //print_list();
    size_t asize;
    size_t extendsize;
    void *bp;
    if(size == 0)   return NULL;
    if(size <= dsize)   asize = 2*dsize;
    else    asize = dsize * ((size + 2*dsize - 1) / dsize);
    if((bp = find_fit(asize)) != NULL){
        //printf("find it\n");
        place(bp, asize);

        //printf("end malloc\n");
        //print_list();
        return bp;
    }
    //printf("no fit \n");
    extendsize = max(asize, chunksize);
    if((bp = extend_heap(extendsize/wsize)) == NULL){
        //printf("5 \n");
        return NULL;
    }

    place(bp, asize);

    //print_list();
    return bp;
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    //printf("mm_free\n");
    if(ptr == NULL || ptr == (void *)-1) return;
    size_t size = get_size(hdrp(ptr));
    put(hdrp(ptr), pack(size, 0));
    put(ftrp(ptr), pack(size, 0));

    //printf("1\n");
    coalesce(ptr);
    //print_list();
    //printf("end free\n");
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    //printf("realloc \n");
    if(ptr == NULL) return mm_malloc(size);
    if(size == 0){
        mm_free(ptr);
        return NULL;
    }

    size_t newsize;
    if(size <= dsize)   newsize = 2*dsize;
    else    newsize = dsize * ((size + 2*dsize - 1) / dsize);
    size_t oldsize = get_size(hdrp(ptr));

    if(newsize > oldsize){
        size_t next = get_alloc(hdrp(next_blkp(ptr)));
        size_t next_size = get_size(hdrp(next_blkp(ptr)));
        
        if(!next && (next_size + oldsize) > newsize){
            delate(next_blkp(ptr));
            put(hdrp(ptr), pack(oldsize+next_size, 1));
            put(ftrp(ptr), pack(oldsize+next_size, 1));
            return ptr;
        }
        if(next_size == 0){
            mem_sbrk(newsize - oldsize);
            put(hdrp(ptr), pack(newsize, 1));
            put(ftrp(ptr), pack(newsize, 1));
            put(hdrp(next_blkp(ptr)), pack(0, 1));
            return ptr;
        }
        void *p;
        p = mm_malloc(size);
        memcpy(p, ptr, oldsize-dsize);
        mm_free(ptr);
        return(p);
    }

    else{
        if((oldsize - newsize) < (2*dsize))  return ptr;
        put(hdrp(ptr), pack(newsize, 1));
        put(ftrp(ptr), pack(newsize, 1));
        ptr = next_blkp(ptr);
        put(hdrp(ptr), pack(oldsize-newsize, 0));
        put(ftrp(ptr), pack(oldsize-newsize, 0));
        coalesce(ptr);
        return prev_blkp(ptr);
    }

}

