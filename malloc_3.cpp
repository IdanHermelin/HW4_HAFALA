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
    int order;
};


MallocMetaDatta* OrdersArray[ORDERS+1]; //Cells of 0-10

int get_order(size_t size){ //size in bytes
    int order = ORDERS;
    int min_needed_blocks = MAXSIZEOFBLOCK;//(bytes)
    while(size < min_needed_blocks/2 && min_needed_blocks>= MINSIZEOFBLOCK){
        min_needed_blocks = min_needed_blocks/2;
        order--;
    }
    return order; //order is the number of cell in the array we will add the memory
}
bool is_list_initialized = false;

MallocMetaDatta* FindInOrder(int order){
    MallocMetaDatta* iterate = OrdersArray[order];
    while(iterate!= nullptr){
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
        prev_node->order = ORDERS;
        OrdersArray[ORDERS] =  prev_node;


        for(int i=1;i<32;i++){
            MallocMetaDatta* cur_node;
            cur_node->next = nullptr;
            cur_node->prev = prev_node;
            cur_node->address = address;
            cur_node->size = MAXSIZEOFBLOCK;
            prev_node->order = ORDERS;
            prev_node->next = cur_node;

            unsigned long next_address = (long) address;
            next_address += MAXSIZEOFBLOCK;
            address = (void*) next_address;

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

    long second_address = (long)first->address;
    second_address = ((long) first->address)^((long)first->size);
    second->address = (void*)second_address;
    add_to_ordered_list(first,cur_order-1);
    add_to_ordered_list(second,cur_order-1);

    return split_blocks(first,cur_order-1,wanted_order);
}
#1
void* smalloc(size_t size){
    if(size < 0 || size > 100000000){
        return nullptr;
    }
    int order = get_order(size + sizeof(MallocMetaDatta));
    int cur_order = order;
    init_list();
    MallocMetaDatta* min_block;
    min_block->address = nullptr;
    int order_of_min_block;
    while(cur_order<=ORDERS){
        //ADD: find the minimal address that is free
        MallocMetaDatta* candidate_block = FindInOrder(cur_order);
        if(candidate_block!= nullptr){
            if(min_block->address == nullptr){
                min_block = candidate_block;
                order_of_min_block = cur_order;
            }
            else{
                if((long)candidate_block->address <= (long)min_block->address){
                    min_block = candidate_block;
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
MallocMetaDatta* find_address_in_array (void* p){
    for(int i=0;i<ORDERS+1 ;i++){
        MallocMetaDatta* Iterate = OrdersArray[i];
        while(Iterate!= nullptr){
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

void merge_buddies (MallocMetaDatta* cur_block, int cur_order){
    if(cur_order == ORDERS) return;
    MallocMetaDatta* buddy = find_address_in_array(get_buddy_address(cur_block));
    if(!buddy->is_free) return;

    MallocMetaDatta* union_block;
    union_block->size = cur_block->size*2;
    union_block->address = min_address(cur_block->address,buddy->address);
    union_block->is_free = true;
    union_block->order = cur_order+1;
    remove_node(cur_block,cur_order);
    remove_node(buddy,cur_order);
    add_to_ordered_list(union_block,cur_order+1);
    merge_buddies(union_block,cur_order+1);
}

#3
void sfree(void* p){
    if(p == nullptr){
        return;
    }
    MallocMetaDatta* wanted_block = find_address_in_array(p);
    if(wanted_block == nullptr) return;
    if(wanted_block->is_free == true) return;
    wanted_block->is_free = true;
    merge_buddies(wanted_block,wanted_block->order);
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