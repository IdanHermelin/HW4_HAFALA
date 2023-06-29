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
    void* address;
    size_t size;
    bool is_free = true;
    MallocMetaDatta* next;
    MallocMetaDatta* prev;
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

MallocMetaDatta* metaData_first = nullptr;
MallocMetaDatta* mettaData_last = nullptr;

MallocMetaDatta* OrdersArray[ORDERS+1]; //Cells of 0-10

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

void* FindInOrder(int order){
    MallocMetaDatta* iterate = OrdersArray[order];
    while(iterate!= nullptr){
        if(iterate->is_free){
            iterate->is_free = false;
            return iterate->address;
        }
        iterate = iterate->next;
    }
    return nullptr;
}

void init_list(){
    if(!is_list_initialized){
        void* currentAddress = sbrk(0);
        unsigned long currentAddressLong = (long)(currentAddress);
        unsigned long toReduce = currentAddressLong % (128 * 1024 * 32);
        unsigned long correct_address = currentAddressLong + (128 * 1024 * 32) - toReduce;
        void* address = (void*) correct_address;

        MallocMetaDatta* prev_node;
        prev_node->next = nullptr;
        prev_node->prev = nullptr;
        prev_node->address = address;
        prev_node->size = MAXSIZEOFBLOCK;

        OrdersArray[ORDERS] =  prev_node;


        for(int i=1;i<32;i++){
            MallocMetaDatta* cur_node;
            cur_node->next = nullptr;
            cur_node->prev = prev_node;
            cur_node->address = address;
            cur_node->size = MAXSIZEOFBLOCK;

            prev_node->next = cur_node;

            unsigned long next_address = (long) address;
            next_address += MAXSIZEOFBLOCK;
            address = (void*) next_address;

            prev_node = cur_node;
        }
    }
    is_list_initialized = true;
}

#1
void* smalloc(size_t size){
    if(size < 0 || size > 100000000){
        return nullptr;
    }
    int order = get_order(size + sizeof(MallocMetaDatta));
    int cur_order = order;
    init_list();
    bool block_found = false;

    if(order == cur_order){
        if(FindInOrder(ORDERS)!= nullptr){
            return FindInOrder(ORDERS);
        }
    }
    while(cur_order<=ORDERS && !block_found){
        //ADD: find the minimal address that is free
        if(cur_order == order){
            if(FindInOrder(cur_order)!= nullptr){
                return FindInOrder(cur_order);
            }
        }
        else{
            void* address = FindInOrder(cur_order)
            if(addrees!= nullptr){
                //split to 2 blocks and remove it from the top level also move the 2 block to lower level
            }
        }

        cur_order++;

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