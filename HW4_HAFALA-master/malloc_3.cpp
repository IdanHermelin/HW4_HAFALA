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
#include <sys/mman.h>
#include <fcntl.h>
#include "math.h"
#include <cstdlib>
#include <ctime>
#include <cstdint>

#define MINSIZEOFBLOCK 128
#define MAXSIZEOFBLOCK 128*1024 //128KB
#define ORDERS 10

uintptr_t size_of_block = MAXSIZEOFBLOCK;
struct MallocMetaDatta{
    int32_t cookie;
    void* address;
    size_t size;
    bool is_free;
    MallocMetaDatta* next;
    MallocMetaDatta* prev;
    int order;
};

int32_t global_cookie;
MallocMetaDatta* OrdersArray[ORDERS+1]; //Cells of 0-10
MallocMetaDatta* first_mmap_block = nullptr;
MallocMetaDatta* last_mmap_block = nullptr;

bool check_cookie(MallocMetaDatta* checked_block){
    if(checked_block->cookie!= global_cookie){
        exit(0xdeadbeef);
    }
    return true;
}

MallocMetaDatta* get_MallocMettaData(void* p){
    MallocMetaDatta* to_return = (MallocMetaDatta*) ((char*)p - sizeof(MallocMetaDatta));
    return to_return;
}


int get_order(size_t size){ //size in bytes
    int order = ORDERS;
    int min_needed_blocks = MAXSIZEOFBLOCK ;//(bytes)
    while(size < min_needed_blocks/2 -sizeof(MallocMetaDatta) && min_needed_blocks>= MINSIZEOFBLOCK){
        min_needed_blocks = min_needed_blocks/2;
        order--;
    }
    return order; //order is the number of cell in the array we will add the memory
}
bool is_list_initialized = false;

MallocMetaDatta* FindInOrder(int order){
    MallocMetaDatta* iterate = OrdersArray[order];
    while(iterate!= nullptr){
        check_cookie(iterate); //check for all the blocks in the current order
        if(iterate->is_free){
            iterate->is_free = false;
            return iterate;
        }
        iterate = iterate->next;
    }
    return nullptr;
}

void init_list(){
    if(!is_list_initialized){
        srand(static_cast <unsigned>(time(0)));
        global_cookie = rand() % (INT32_MAX);

        void* currentAddress = sbrk(0);

        uintptr_t currentAddressLong = reinterpret_cast<uintptr_t>(currentAddress);
        uintptr_t toReduce = currentAddressLong % (128 * 1024 * 32);
        //unsigned long correct_address = currentAddressLong + (128 * 1024 * 32) - toReduce;
        //void* address = (void*) correct_address;
        currentAddress = sbrk((128 * 1024 * 32) - toReduce);

        MallocMetaDatta* prev_node = get_MallocMettaData(currentAddress);
        prev_node->next = nullptr;
        prev_node->prev = nullptr;
        prev_node->address = currentAddress;
        prev_node->is_free = true;
        prev_node->size = MAXSIZEOFBLOCK;
        prev_node->order = ORDERS;
        prev_node->cookie = global_cookie;
        OrdersArray[ORDERS] =  prev_node;

//        uintptr_t next_address = reinterpret_cast<uintptr_t>(currentAddress);
//        next_address += size_of_block;
//        currentAddress = reinterpret_cast<void*>(next_address);
        currentAddress = sbrk(MAXSIZEOFBLOCK);

        for(int i=1;i<32;i++){
            
            MallocMetaDatta* cur_node = get_MallocMettaData(currentAddress);
            cur_node->next = nullptr;
             cur_node->prev = prev_node;
            cur_node->address = currentAddress;
            cur_node->is_free = true;
            cur_node->size = MAXSIZEOFBLOCK;
            cur_node->cookie = global_cookie;
            prev_node->order = ORDERS;
            prev_node->next = cur_node;

//            uintptr_t next_address = reinterpret_cast<uintptr_t>(currentAddress);
//            next_address += MAXSIZEOFBLOCK;
            currentAddress = sbrk(MAXSIZEOFBLOCK);
//            currentAddress = reinterpret_cast<void*>(next_address);

            prev_node = cur_node;
        }
    }
    is_list_initialized = true;
}
void remove_node (MallocMetaDatta* block, int cur_order){
    if(block == OrdersArray[cur_order]){
        if(block->next != nullptr){
            OrdersArray[cur_order] = block->next;
            block->next->prev = nullptr;
        }
        else{
            OrdersArray[cur_order] = nullptr;
        }
    }
    else{
        if(block->next != nullptr){
            block->prev->next = block->next;
            block->next->prev = block->prev;
        }
        else{
            block->prev->next = nullptr;
        }
    }
    block->prev = nullptr;
    block->next = nullptr;
}

void add_to_ordered_list(MallocMetaDatta* added_block, int order){
    if(OrdersArray[order] == nullptr){
        OrdersArray[order] = added_block;
        added_block->prev = nullptr;
        added_block->next = nullptr;
        return;
    }
    MallocMetaDatta* iterate = OrdersArray[order];
    while(iterate->next != nullptr){
        iterate = iterate->next;
    }
    iterate->next = added_block;
    added_block->prev = iterate;
    added_block->next = nullptr;
}

void* split_blocks(MallocMetaDatta* min_block, int cur_order, int wanted_order){
    if(cur_order == wanted_order){
        return min_block->address;
    }
    remove_node(min_block,cur_order);
    MallocMetaDatta* first = min_block;
    first->size = first->size/2;
    MallocMetaDatta* second = first;
    first->order = cur_order-1;
    second->order = cur_order-1;
    first->is_free = false;
    second->is_free = true;
    first->cookie = global_cookie;
    second->cookie = global_cookie;

    long second_address = (long)first->address;
    second_address = ((long) first->address)^((long)first->size);
    second->address = (void*)second_address;
    add_to_ordered_list(first,cur_order-1);
    add_to_ordered_list(second,cur_order-1);

    return split_blocks(first,cur_order-1,wanted_order);
}
void* allocate_mmap_block(size_t size){
    void* new_allocate = mmap(nullptr,size+sizeof(MallocMetaDatta),
                              PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1,0);
    if(new_allocate == MAP_FAILED){
        return nullptr;
    }
    MallocMetaDatta* newAllocatedBlock = (MallocMetaDatta*) new_allocate;
    newAllocatedBlock->size = size;
    newAllocatedBlock->is_free = false;
    newAllocatedBlock->next = nullptr;
    newAllocatedBlock->cookie = global_cookie;

    if(first_mmap_block == nullptr){
        first_mmap_block = newAllocatedBlock;
        last_mmap_block = newAllocatedBlock;
        last_mmap_block->prev = nullptr;
        first_mmap_block->prev = nullptr;
    }
    else{
        last_mmap_block->next = newAllocatedBlock;
        newAllocatedBlock->prev = last_mmap_block;
        last_mmap_block = last_mmap_block->next;
    }
    newAllocatedBlock->address = (void*) ((long)new_allocate+ sizeof (MallocMetaDatta));
    return newAllocatedBlock->address;
}


MallocMetaDatta* find_mmap_block(void* p){
    MallocMetaDatta* iterate = first_mmap_block;
    while(iterate != nullptr){
        check_cookie(iterate);
        if(iterate->address == p){
            return iterate;
        }
        iterate = iterate->next;
    }
    return nullptr;
}


void free_mmap_block(MallocMetaDatta* to_free){

    if(to_free == first_mmap_block){
        if(to_free->next == nullptr){
            first_mmap_block = nullptr;
            last_mmap_block = nullptr;
        }
        else{
            first_mmap_block = first_mmap_block->next;
        }
    }
    else{
        if(to_free == last_mmap_block){
            last_mmap_block = last_mmap_block->prev;
            last_mmap_block->next = nullptr;
        }
        else{
            to_free->prev->next = to_free->next;
            to_free->next->prev = to_free->prev;
        }
    }
    munmap(to_free,sizeof(MallocMetaDatta) + to_free->size);
}



void* smalloc(size_t size){
    if(size > MAXSIZEOFBLOCK){ //we need to use mmap
        return allocate_mmap_block(size);
    }
    if(size < 0 || size > 100000000){
        return nullptr;
    }
    int order = get_order(size + sizeof(MallocMetaDatta));
    int cur_order = order;
    init_list();
    MallocMetaDatta* min_block;
    void* target_address = nullptr;
    int order_of_min_block;
    while(cur_order<=ORDERS){
        //ADD: find the minimal address that is free
        MallocMetaDatta* candidate_block = FindInOrder(cur_order);
        if(candidate_block!= nullptr){
            if(target_address == nullptr){
                min_block = candidate_block;
                target_address = candidate_block->address;
                order_of_min_block = cur_order;
            }
            else{
                if((long)candidate_block->address <= (long)target_address){
                    min_block = candidate_block;
                    target_address = candidate_block->address;
                    order_of_min_block = cur_order;
                }
            }
        }
        cur_order++;
    }
    if(min_block->address == nullptr) return nullptr;
    //splitting while the current order isn't the requested order for the block:
    return split_blocks(min_block, order_of_min_block, order);
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
MallocMetaDatta* find_address_in_array (void* p){
    for(int i=0;i<ORDERS+1 ;i++){
        MallocMetaDatta* Iterate = OrdersArray[i];
        while(Iterate!= nullptr){
            check_cookie(Iterate);
            if(Iterate->address == p) return Iterate;
        }
    }
    return nullptr;
}

void* get_buddy_address(MallocMetaDatta* block){
    long buddy_address = ((long) block->address)^((long)block->size);
    return (void*)buddy_address;
}

void* min_address (void* addr1, void* addr2){
    if((long)addr1 <= (long) addr2) return addr1;
    return addr2;
}

void* merge_buddies (MallocMetaDatta* cur_block, int cur_order,int max_order,bool cond_needed){
    if(cur_order == max_order) return cur_block->address;
    MallocMetaDatta* buddy = find_address_in_array(get_buddy_address(cur_block));
    if(cond_needed && !buddy->is_free) return cur_block->address;

    MallocMetaDatta* union_block = cur_block;
    union_block->size = cur_block->size*2;
    union_block->address = min_address(cur_block->address,buddy->address);
    union_block->is_free = true;
    union_block->order = cur_order+1;
    union_block->cookie = global_cookie;
    remove_node(cur_block,cur_order);
    remove_node(buddy,cur_order);
    add_to_ordered_list(union_block,cur_order+1);
    return merge_buddies(union_block,cur_order+1,max_order,cond_needed);
}


void sfree(void* p){
    if(p == nullptr){
        return;
    }
    MallocMetaDatta* is_mmap = find_mmap_block(p);
    if(is_mmap != nullptr){
        if(is_mmap->is_free){
            return;
        }
        free_mmap_block(is_mmap);
        return;
    }
    MallocMetaDatta* wanted_block = find_address_in_array(p);
    if(wanted_block == nullptr) return;
    if(wanted_block->is_free == true) return;
    wanted_block->is_free = true;
    merge_buddies(wanted_block,wanted_block->order,ORDERS,true);
}


void* srealloc(void* oldp, size_t size){
    if(size == 0 || size > 100000000){
        return nullptr;
    }
    if(oldp == nullptr){
        void* newAllocate = smalloc(size);
        return newAllocate;
    }
    int order_of_size = get_order(size);
    size_t oldpSize;
    //check when size is not for mmap
    if(size <= MAXSIZEOFBLOCK){
        MallocMetaDatta* p_block = find_address_in_array(oldp);
        if(p_block != nullptr){
            oldpSize = p_block->size - sizeof(MallocMetaDatta);
            if(oldpSize >= size){
                return oldp;
            }
            else{
                return merge_buddies(p_block,p_block->order,order_of_size,false);
            }
        }
    }

    //check when size is for mmap
    MallocMetaDatta* res = find_mmap_block(oldp);
    oldpSize = res->size;
    if(res->size == size){
        return res->address;
    }
    //oldp is not enough
    void* newAllocation = smalloc(size);
    if(newAllocation == nullptr){
        return nullptr;
    }
    memmove(newAllocation,oldp,oldpSize);
    sfree(oldp);
    return newAllocation;
}


size_t _num_free_blocks(){
    size_t counter=0;
    for(int cur_order = 0 ; cur_order<=ORDERS ;cur_order++){
        MallocMetaDatta* tmp = OrdersArray[cur_order];
        while(tmp != nullptr){
            check_cookie(tmp);
            if(tmp->is_free){
                counter++;
            }
            tmp = tmp->next;
        }
    }
    return counter;
}

size_t _num_free_bytes(){
    size_t counter=0;
    for(int cur_order = 0 ; cur_order<=ORDERS ;cur_order++){
        MallocMetaDatta* tmp = OrdersArray[cur_order];
        while(tmp != nullptr){
            check_cookie(tmp);
            if(tmp->is_free){
                counter+= tmp->size - sizeof(MallocMetaDatta);
            }
            tmp = tmp->next;
        }
    }
    return counter;
}

size_t _num_allocated_blocks(){
    size_t counter=0;
    for(int cur_order = 0 ; cur_order<=ORDERS ;cur_order++){
        MallocMetaDatta* tmp = OrdersArray[cur_order];
        while(tmp != nullptr){
            check_cookie(tmp);
            counter++;
            tmp = tmp->next;
        }
    }
    //iterate for the mmap blocks:
    MallocMetaDatta* iterate = first_mmap_block;
    while(iterate != nullptr){
        check_cookie(iterate);
        counter++;
        iterate = iterate->next;
    }
    return counter;
}

size_t _num_allocated_bytes() {
    size_t counter=0;
    for(int cur_order = 0 ; cur_order<=ORDERS ;cur_order++){
        MallocMetaDatta* tmp = OrdersArray[cur_order];
        while(tmp != nullptr){
            check_cookie(tmp);
            counter+= tmp->size - sizeof(MallocMetaDatta);
            tmp = tmp->next;
        }
    }
    //iterate for the mmap blocks:
    MallocMetaDatta* iterate = first_mmap_block;
    while(iterate != nullptr){
        check_cookie(iterate);
        counter += iterate->size;
        iterate = iterate->next;
    }
    return counter;
}

size_t _num_meta_data_bytes(){
    return sizeof(MallocMetaDatta)* _num_allocated_blocks();
}

size_t _size_meta_data(){
    return sizeof(MallocMetaDatta);
}

