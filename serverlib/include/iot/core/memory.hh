////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include "types.hh"
#include <limits>
#include <assert.h>

namespace rohit {

class memory_store {
private:
    uint8_t * const store;
    size_t count;
    const size_t capacity;


    inline bool in_store(const void *pvalue, const size_t alloc_size) const {
        const uint8_t *pivalue = (uint8_t *)pvalue;
        return pivalue >= store && pivalue < store + capacity * alloc_size;
    }

    inline bool final_free(const void *pvalue, const size_t alloc_size) {
        assert(count != 0);

        if (in_store(pvalue, alloc_size)) {
            --count;
            return true;
        }

        return false;
    }

    friend class fixed_memory;
    
public:
    inline memory_store(const size_t alloc_size, const size_t capacity)
        :   store(capacity ? new uint8_t[capacity * alloc_size]: nullptr),
            count(0),
            capacity(capacity) {}

    inline size_t get_capacity() const { return capacity; }

}; // class memory_store


class fixed_memory {
public:
    static constexpr size_t min_capacity = 256;
    static constexpr size_t max_store = 23;
private:
    static constexpr size_t null_index = std::numeric_limits<uint32_t>::max();

    const size_t alloc_size;
    // Initially all is free we will start with used index
    // keep on increasing it once it is filled,
    // free_start_index will be used
    uint32_t free_start_index;
    uint32_t *free_linked_index;
    uint32_t used_index;
    memory_store *current_store;
    
    size_t to_free_count;
    memory_store *store_to_free[max_store];

    uint8_t *get_memory();

public:
    inline fixed_memory(const size_t alloc_size, const size_t initial_capacity = min_capacity)
            :   alloc_size(alloc_size),
                free_start_index(null_index),
                free_linked_index(new uint32_t[initial_capacity]),
                used_index(0),
                current_store(new memory_store(alloc_size, initial_capacity)),
                to_free_count(0),
                store_to_free() {
        assert(alloc_size >=4); // "Allocation size must be atleast 4"
        assert(alloc_size == 4 || alloc_size % 4 == 0); //"Allocation size must be aligned to 4"
    }

    template <typename T, typename... ARGS>
    inline T *alloc(ARGS&... args) {
        assert((sizeof(T) + 3) & (~3) == alloc_size); // "Wrong alloc function allocation size must match"
        uint8_t *memptr = get_memory();
        return new (memptr) T(args...);
    } // T *alloc

    template <typename T>
    inline void free(const T *value) {
        assert((sizeof(T) + 3) & (~3) == alloc_size); // "Wrong free function allocation size must match"
        const void *pvalue = (void *)value;
        free(pvalue);
    }

    void free(const void *pvalue);

}; // class fixed_memory

class memory {
public:
    static constexpr size_t max_allocation_size = 1024;
    static constexpr size_t max_chunk = max_allocation_size >> 2;

private:
    fixed_memory mem_array[max_chunk];
    
public:
    memory() : mem_array {
        {   4}, {   8}, {  12}, {  16}, {  20}, {  24}, {  28}, {  32}, {  36}, {  40}, {  44}, {  48},
        {  52}, {  56}, {  60}, {  64}, {  68}, {  72}, {  76}, {  80}, {  84}, {  88}, {  92}, {  96},
        { 100}, { 104}, { 108}, { 112}, { 116}, { 120}, { 124}, { 128}, { 132}, { 136}, { 140}, { 144},
        { 148}, { 152}, { 156}, { 160}, { 164}, { 168}, { 172}, { 176}, { 180}, { 184}, { 188}, { 192},
        { 196}, { 200}, { 204}, { 208}, { 212}, { 216}, { 220}, { 224}, { 228}, { 232}, { 236}, { 240},
        { 244}, { 248}, { 252}, { 256}, { 260}, { 264}, { 268}, { 272}, { 276}, { 280}, { 284}, { 288},
        { 292}, { 296}, { 300}, { 304}, { 308}, { 312}, { 316}, { 320}, { 324}, { 328}, { 332}, { 336},
        { 340}, { 344}, { 348}, { 352}, { 356}, { 360}, { 364}, { 368}, { 372}, { 376}, { 380}, { 384},
        { 388}, { 392}, { 396}, { 400}, { 404}, { 408}, { 412}, { 416}, { 420}, { 424}, { 428}, { 432},
        { 436}, { 440}, { 444}, { 448}, { 452}, { 456}, { 460}, { 464}, { 468}, { 472}, { 476}, { 480},
        { 484}, { 488}, { 492}, { 496}, { 500}, { 504}, { 508}, { 512}, { 516}, { 520}, { 524}, { 528},
        { 532}, { 536}, { 540}, { 544}, { 548}, { 552}, { 556}, { 560}, { 564}, { 568}, { 572}, { 576},
        { 580}, { 584}, { 588}, { 592}, { 596}, { 600}, { 604}, { 608}, { 612}, { 616}, { 620}, { 624},
        { 628}, { 632}, { 636}, { 640}, { 644}, { 648}, { 652}, { 656}, { 660}, { 664}, { 668}, { 672},
        { 676}, { 680}, { 684}, { 688}, { 692}, { 696}, { 700}, { 704}, { 708}, { 712}, { 716}, { 720},
        { 724}, { 728}, { 732}, { 736}, { 740}, { 744}, { 748}, { 752}, { 756}, { 760}, { 764}, { 768},
        { 772}, { 776}, { 780}, { 784}, { 788}, { 792}, { 796}, { 800}, { 804}, { 808}, { 812}, { 816},
        { 820}, { 824}, { 828}, { 832}, { 836}, { 840}, { 844}, { 848}, { 852}, { 856}, { 860}, { 864},
        { 868}, { 872}, { 876}, { 880}, { 884}, { 888}, { 892}, { 896}, { 900}, { 904}, { 908}, { 912},
        { 916}, { 920}, { 924}, { 928}, { 932}, { 936}, { 940}, { 944}, { 948}, { 952}, { 956}, { 960},
        { 964}, { 968}, { 972}, { 976}, { 980}, { 984}, { 988}, { 992}, { 996}, {1000}, {1004}, {1008},
        {1012}, {1016}, {1020}, {1024} } {}

    template <typename T, typename... ARGS>
    inline T *alloc(ARGS&... args) {
        constexpr size_t alloc_size = (sizeof(T) + 3) & (~3);
        constexpr size_t chunk_index = alloc_size >> 2;

        if constexpr (alloc_size > max_allocation_size) return new T(args...);
        else {
            return mem_array[chunk_index].alloc<T, ARGS...>(args...);
        }
    }

    template <typename T>
    inline void free(T *value) {
        constexpr size_t alloc_size = (sizeof(T) + 3) & (~3);
        constexpr size_t chunk_index = alloc_size >> 2;

        if constexpr (alloc_size > max_allocation_size) delete &value;
        else {
            mem_array[chunk_index].free(value);
        }
    }
}; // class memory

extern memory allocator;

} // namespace rohit