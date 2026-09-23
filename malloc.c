#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h> 
#include <stdio.h> 


#define META_SIZE sizeof(struct block_meta)




struct block_meta{

    size_t size;
    struct block_meta * next;
    int free;

};



void * global_base = NULL;


struct block_meta * findFreeBlock(struct block_meta ** last, size_t size){

    struct block_meta * current = global_base;

    while(current && (current -> free == 0 || current -> size < size)){
        *last = current;
        current = current -> next;
    }

    return current;
}


struct block_meta * requestSpace(struct block_meta * last, size_t size){
    struct block_meta * block;
    void * p = sbrk(0);

    void * request = sbrk(size + META_SIZE);

    if( request == (void *) - 1){
        return NULL;
    }

    assert(p == request);

    block = p;

    if(last != NULL){
        last -> next = block;
    }

    block -> size = size;
    block -> next = NULL;
    block -> free = 0;

    return block;

}



void * myMalloc2(size_t size){
    if(size <= 0 ){
        return NULL;
    }

    struct block_meta * block;


    if(global_base == NULL){
        block = requestSpace(NULL,size);

        if(block == NULL){
            return NULL;
        }

    global_base = block;

    }
    else{
        struct block_meta *last = global_base;
        block = findFreeBlock(&last,size);
        if(block == NULL){
            block = requestSpace(last,size);
            if(block == NULL){
                return NULL;
            }

        }
        else{
            block->free = 0;
        }
    }

    return (block + 1);

}



struct block_meta * getMeta(void * pointer){
    if(pointer == NULL){
        return NULL;
    }

    return (struct block_meta *)pointer - 1;
}



void free(void * pointer){

    if(pointer == NULL){
        return;
    }

    struct block_meta * meta = getMeta(pointer);
    
    assert(meta->free == 0);

    meta->free = 1;

}


void * myCalloc(size_t number, size_t size){

    if(size != 0 && number > SIZE_MAX / size){
        return NULL;
    }

    size_t total = number * size;


    void * ptr = myMalloc2(total);

    memset(ptr,0,total);

    return ptr;
}
 

void * myRealloc(void * ptr, size_t size){

    if(ptr == NULL){
        void * block = myMalloc2(size);
        return block;
    }


    struct block_meta * block = getMeta(ptr);


    if(block->size > size){

        return ptr;

    }

    void * newptr = myMalloc2(size);

    if(newptr == NULL){
        return NULL;
    }

    memcpy(newptr,ptr,block->size);

    free(ptr);

    return newptr;
    
}



void splitBlock(struct block_meta *block, size_t size) {
    size_t remaining = block->size - size;

    if (remaining <= META_SIZE) {
        return;
    }

    struct block_meta *newblock =
        (struct block_meta *)((char *)(block + 1) + size);

    newblock->size = remaining - META_SIZE;
    newblock->next = block->next;
    newblock->free = 1;

    block->size = size;
    block->next = newblock;
}

void *memoryeffeciantMalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    struct block_meta *block;

    if (global_base == NULL) {
        block = requestSpace(NULL, size);

        if (block == NULL) {
            return NULL;
        }

        global_base = block;
    } else {
        struct block_meta *last = global_base;
        block = findFreeBlock(&last, size);

        if (block == NULL) {
            block = requestSpace(last, size);

            if (block == NULL) {
                return NULL;
            }
        } else {
            splitBlock(block, size);
            block->free = 0;
        }
    }

    return block + 1;
}


void mergeBlocks(struct block_meta * block){
    struct block_meta * current = block;
    struct block_meta * prev = NULL;
    while(current &&  current->next != NULL){

        prev = current;
        current = current->next;

        if((current->free == 1 && prev->free == 1) && ((char *)(prev + 1) + prev->size == (char *)current)){

            prev -> next = current -> next;
            prev -> size += META_SIZE + current -> size;




        current = prev;



        }


    }
}




int main(void){

    int * number = memoryeffeciantMalloc(sizeof(int) * 2);

    number[0] = 1;
    number[1] =  3;

    printf("this is the number %d  \n", number[0]);

    number = myRealloc(number, sizeof(int) * 10);

    number[5] = 5;

    printf("this is the number %d  \n", number[5]);


}

