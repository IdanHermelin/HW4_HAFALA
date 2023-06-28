//
// Created by student on 6/27/23.
//
//
// Created by student on 6/26/23.
//
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#define MINSIZEOFBLOCK 128
#define MAXSIZEOFBLOCK 128*1024 //128KB
#define ORDERS 10

struct MallocMetaDatta{
    int relative_address;
    size_t size;
    bool is_free = true;
    MallocMetaDatta* next_baddy_block;
    MallocMetaDatta* prev_baddy_block;
    MallocMetaDatta* parent;
    MallocMetaDatta* son;
};
/*
struct ListedMalloc{
    size_t num_of_blocks;
    size_t size_of_blocks;
    MallocMetaDatta* first = nullptr;
    MallocMetaDatta* last = nullptr;
    ListedMalloc* next_order;
    ListedMalloc* prev_order;
    size_t number_of_nodes = 0; //maybe
};
*/
//ListedMalloc * ListedArray[ORDERS];

MallocMetaDatta* metaData_first = nullptr;
MallocMetaDatta* mettaData_last = nullptr;

size_t get_order(size_t size){ //size in bytes
    int order = ORDERS;
    int min_needed_blocks = MAXSIZEOFBLOCK;//(bytes)
    while(size < min_needed_blocks/2 && min_needed_blocks>= MINSIZEOFBLOCK){
        min_needed_blocks = min_needed_blocks/2;
        order--;
    }
    return order; //order is the number of cell in the array we will add the memory
}
bool is_list_initialized = false;
void init_list(){
    int address = 0;
    if(!is_list_initialized){
        for(int i=0;i<32;i++){
            MallocMetaDatta* node = (MallocMetaDatta*) malloc(sizeof(MAXSIZEOFBLOCK));
            node->next_baddy_block = nullptr;
            node->prev_baddy_block = nullptr;
            node->parent = nullptr;
            node->son = nullptr;
            node->relative_address = address;
            //node->size = MAXSIZEOFBLOCK - sizeof(MallocMetaDatta);
            node->size = MAXSIZEOFBLOCK;
            if(i==0){
                metaData_first = node;
            }
            if(i==31){
                mettaData_last == node;
            }
            address+=MAXSIZEOFBLOCK;
        }
    }
    is_list_initialized = true;
}

#1
void* smalloc(size_t size){
    if(size < 0 || size > 100000000){
        return nullptr;
    }
    int cur_order = ORDERS-1;
    init_list();
    bool block_found = false;
    int order = get_order(size + sizeof(MallocMetaDatta));
    MallocMetaDatta* cur_parent = metaData_first;
    MallocMetaDatta* cur_son;
    if(order == ORDERS-1){

    }

    while(cur_order > order && !block_found){
        if(cur_parent->son){
            cur_son = cur_parent->son;
        }
        else{
            MallocMetaDatta* new_son;
            MallocMetaDatta* new_buddy_son;
            cur_parent->son = new_son;
            new_son->size = cur_parent->size/2;
            new_son->parent = cur_parent;
            new_son->next_baddy_block = new_buddy_son;
            new_son->relative_address = cur_parent->relative_address;
            new_buddy_son->relative_address = cur_parent->relative_address + cur_parent->size/2;
            //continue adding fields
        }
        cur_order--;
    }
}
#2
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
#3
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
#4
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

#5
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
#6
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
#7
size_t _num_allocated_blocks(){
    MallocMetaDatta* tmp = metaData_first;
    size_t counter=0;
    while(tmp != nullptr){
        counter++;
        tmp = tmp->next;
    }
    return counter;
}
#8
size_t _num_allocated_bytes() {
    MallocMetaDatta *tmp = metaData_first;
    size_t counter = 0;
    while (tmp != nullptr) {
        counter += tmp->size;
        tmp = tmp->next;
    }
    return counter;
}
#9
size_t _num_meta_data_bytes(){
    return sizeof(MallocMetaDatta)* _num_allocated_blocks();
}
#10
size_t _size_meta_data(){
    return sizeof(MallocMetaDatta);
}