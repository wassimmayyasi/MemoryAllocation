#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

struct space {
    size_t size;
    struct space *nextSpace;
};

struct space *heapStart, *heapEnd;
struct space *freeListStart;

void *mymalloc(size_t size) {
    if(size == 0)   
        return NULL;
//Look if we have a freeListStart
    if(freeListStart != NULL) {
        struct space *temporaryBlock = freeListStart;
        while(temporaryBlock != NULL) {
            if(temporaryBlock == freeListStart && temporaryBlock->size >= size){
                freeListStart = temporaryBlock->nextSpace;
                return (void*)(temporaryBlock + 1);
            }
            if(temporaryBlock->nextSpace != NULL && temporaryBlock->nextSpace->size >= size) {
                struct space *saveBlock = temporaryBlock->nextSpace;
                temporaryBlock->nextSpace = temporaryBlock->nextSpace->nextSpace;
                saveBlock->nextSpace = NULL;
                return (void*)(saveBlock + 1);
            }
            temporaryBlock = temporaryBlock->nextSpace;
        }
    }
// if no free used space, make a new space at the end of the heap
    size_t totalSize = size + sizeof(struct space);
    if(totalSize % sizeof(long) != 0) //for aligning with sizeof(long)
        totalSize = totalSize + (sizeof(long) - totalSize % sizeof(long));

    struct space *newSpaceHeader = sbrk(totalSize);
    if(newSpaceHeader == (void*)-1)
        return NULL;
    newSpaceHeader->size = size;
	newSpaceHeader->nextSpace = NULL;

    if(heapStart == NULL)
        heapStart = newSpaceHeader;
    if(heapEnd != NULL) 
        heapEnd->nextSpace = newSpaceHeader;
    heapEnd = newSpaceHeader;
    return (void*)(newSpaceHeader + 1);
}

void *mycalloc(size_t nmemb, size_t size) {
    if(size == 0 || nmemb == 0)
      return NULL;
    
    void *addedSpace = mymalloc(nmemb*size);
    if(!addedSpace)
       return NULL;

    memset(addedSpace, 0, nmemb*size);
    return addedSpace;
}

void myfree(void *ptr) {
    if(ptr == NULL)
        return;
    struct space *ptrHeader = (struct space*)ptr - 1;
    void *currentProgBreak = sbrk(0);

    if((char*)ptr + ptrHeader->size == currentProgBreak) { //if it was the last block (our current program break is on it)
        if(heapStart == heapEnd) { //its just 1 block of memory in the entire heap
            heapStart = NULL;
            heapEnd = NULL;
        } else {  
            struct space *temporary = ptrHeader;
            while(temporary != NULL) {
                if(temporary->nextSpace == heapEnd) { //to assign the before last memory block to NULL
                    temporary->nextSpace = NULL;
                    heapEnd = temporary;
                }
                temporary = temporary->nextSpace;
            }
        }
        sbrk(0 - sizeof(struct space) - ptrHeader->size);
        return;
    }
    //if its not the last memory space, we want to store it in freeList to reuse
    if(freeListStart == NULL) { //empty freeList so we can place it as the first node
        ptrHeader->nextSpace = NULL;
        freeListStart = ptrHeader;
        return;
    }
    //for non empty freeList, we will make the recently freed memory the new start
    struct space *temporary = freeListStart;
    freeListStart = ptrHeader;
    freeListStart->nextSpace = temporary;
}

void *myrealloc(void *ptr, size_t size) {
    if(ptr == NULL)
        return mymalloc(size);
    
    struct space *ptrHeader = (struct space*)ptr - 1;
    if(ptrHeader->size >= size)
       return ptr;

    void *newSpace = mymalloc(size);
    if(newSpace != NULL) {
        memcpy(newSpace, ptr, ptrHeader->size);
        myfree(ptr);
    }
    return newSpace;
}

/*
 * Enable the code below to enable system allocator support for your allocator.
 * Doing so will make debugging much harder (e.g., using printf may result in
 * infinite loops).
 */
#if 1
void *malloc(size_t size) { return mymalloc(size); }
void *calloc(size_t nmemb, size_t size) { return mycalloc(nmemb, size); }
void *realloc(void *ptr, size_t size) { return myrealloc(ptr, size); }
void free(void *ptr) { myfree(ptr); }
#endif