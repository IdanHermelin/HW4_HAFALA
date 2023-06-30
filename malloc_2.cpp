//
// Created by student on 6/26/23.
//
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstring>




struct MallocMetaDatta{
    size_t size;
    bool is_free;
    MallocMetaDatta* next;
    MallocMetaDatta* prev;
};

MallocMetaDatta* metaData_first = nullptr;
MallocMetaDatta* mettaData_last = nullptr;





void* smalloc(size_t size){
    if(size <= 0 || size > 100000000){
        return nullptr;
    }
    MallocMetaDatta* tmp = metaData_first;
    while(tmp != nullptr){
        if(tmp->size >= size && tmp->is_free == true){
            return (void*)((long)tmp+sizeof(MallocMetaDatta));
        }
        tmp = tmp->next;
    }
    void* checkAllocate = sbrk(sizeof(MallocMetaDatta) + size);
    if(checkAllocate == (void*)-1){
        return NULL;
    }
    auto* newAllocate = static_cast<MallocMetaDatta*>(checkAllocate);
    newAllocate->size = size;
    newAllocate->is_free = false;
    newAllocate->next = nullptr;
    if(metaData_first == nullptr){
        newAllocate->prev = nullptr;
        metaData_first = newAllocate;
        mettaData_last = newAllocate;
    }
    else{
        mettaData_last->next = newAllocate;
        newAllocate->prev = mettaData_last;
        mettaData_last = mettaData_last->next;
    }
    return (void*)((long)newAllocate+sizeof(MallocMetaDatta));
}
void* scalloc(size_t num, size_t size){
    if(size == 0 || num == 0){
        return nullptr;
    }
    void* isBlockExist = smalloc(num*size);
    if(isBlockExist != nullptr){
        memset(isBlockExist,0,num*size);
        return isBlockExist;
    }
    return nullptr;
}

void sfree(void* p){
    if(p == nullptr){
        return;
    }
    MallocMetaDatta* tmp = metaData_first;
    while(tmp != nullptr){
        if((void*)(long(tmp) + sizeof(MallocMetaDatta)) == p){
            if(tmp->is_free == true){
                return;
            }
            tmp->is_free = true;
            return;
        }
        tmp = tmp->next;
    }
}
void* srealloc(void* oldp, size_t size){
    if(size == 0 || size > 100000000){
        return nullptr;
    }
    if(oldp == nullptr){
        void* newAllocate = smalloc(size);
        return newAllocate;
    }
    MallocMetaDatta* tmp = metaData_first;
    size_t oldpSize;
    while(tmp != nullptr){
        if((void*)(long(tmp) + sizeof(MallocMetaDatta)) == oldp){
            oldpSize = tmp->size;
            if(tmp->size >= size){
                return oldp;
            }
        }
        tmp = tmp->next;
    }
    void* newAllocation = smalloc(size);
    if(newAllocation == nullptr){
        return nullptr;
    }
    memmove(newAllocation,oldp,oldpSize);
    sfree(oldp);
    return newAllocation;
}

size_t _num_free_blocks(){
    MallocMetaDatta* tmp = metaData_first;
    size_t counter=0;
    while(tmp != nullptr){
        if(tmp->is_free){
            counter++;
        }
        tmp = tmp->next;
    }
    return counter;
}
size_t _num_free_bytes(){
    MallocMetaDatta* tmp = metaData_first;
    size_t counter=0;
    while(tmp != nullptr){
        if(tmp->is_free){
            counter+= tmp->size;
        }
        tmp = tmp->next;
    }
    return counter;
}
size_t _num_allocated_blocks(){
    MallocMetaDatta* tmp = metaData_first;
    size_t counter=0;
    while(tmp != nullptr){
        counter++;
        tmp = tmp->next;
    }
    return counter;
}

size_t _num_allocated_bytes() {
    MallocMetaDatta *tmp = metaData_first;
    size_t counter = 0;
    while (tmp != nullptr) {
        counter += tmp->size;
        tmp = tmp->next;
    }
    return counter;
}

size_t _num_meta_data_bytes(){
    return sizeof(MallocMetaDatta)* _num_allocated_blocks();
}

size_t _size_meta_data(){
    return sizeof(MallocMetaDatta);
}





