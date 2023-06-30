//
// Created by student on 6/26/23.
//
#include <unistd.h>

void* smalloc(size_t size){
    if(size <= 0 || size > 100000000){
        return NULL;
    }
    void* checkAllocate = sbrk(size);
    if(checkAllocate == (void*)-1){
        return NULL;
    }
    return checkAllocate;
}