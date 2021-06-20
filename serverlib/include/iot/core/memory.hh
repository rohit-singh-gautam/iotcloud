////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include "types.hh"
#include <limits>
#include <assert.h>
#include <pthread.h>
#include "config.hh"

namespace rohit {

struct fixed_memory_alloc_info {
    uint16_t alloc_size;
    uint8_t  store_index;
    static constexpr uint8_t default_memory_check = 0xaa;
    // This is unique memory can be utilized for memory check in debug mode
    uint8_t  memory_check;


    constexpr fixed_memory_alloc_info(const uint16_t alloc_size, const uint8_t store_index)
        : alloc_size(alloc_size), store_index(store_index), memory_check(default_memory_check) {}

    constexpr fixed_memory_alloc_info(const fixed_memory_alloc_info &info) 
        : alloc_size(info.alloc_size), store_index(info.store_index), memory_check(info.memory_check) {}

    constexpr fixed_memory_alloc_info operator=(const fixed_memory_alloc_info &rhs) {
        alloc_size = rhs.alloc_size;
        store_index = rhs.store_index;
        memory_check = memory_check;
        return *this;
    }

}  __attribute__((packed));

struct fixed_memory_free_info {
    // This will not be more than 8 bit
    uint64_t store_index:8;
    uint64_t memory_index:56;

    constexpr fixed_memory_free_info(const uint8_t store_index, const uint64_t memory_index)
        : store_index(store_index), memory_index(memory_index) { }

    constexpr fixed_memory_free_info(const fixed_memory_free_info &info) 
        : store_index(info.store_index), memory_index(info.memory_index) {}

    constexpr fixed_memory_free_info operator=(const fixed_memory_free_info &rhs) {
        store_index = rhs.store_index;
        memory_index = rhs.memory_index;
        return *this;
    }

    constexpr bool operator==(const fixed_memory_free_info &rhs) {
        return store_index == rhs.store_index && memory_index == rhs.memory_index;
    }

}  __attribute__((packed));

class fixed_memory {
public:
    static constexpr size_t min_capacity = 8;
    static constexpr size_t max_store = 32;

private:
    static constexpr fixed_memory_free_info null_index = {0xff, 0xffffffffffffff};

    const size_t alloc_size;
    // Initially all is free we will start with used index
    // keep on increasing it once it is filled,
    // free_start_index will be used
    fixed_memory_free_info free_start_index;
    size_t current_capacity;
    size_t last_store_index;
    uint8_t *store_block[max_store];

    pthread_mutex_t lock;

public:
    inline fixed_memory(const size_t alloc_size)
            :   alloc_size(alloc_size + sizeof(fixed_memory_alloc_info)),
                free_start_index(null_index),
                current_capacity(min_capacity >> 1),
                last_store_index(-1),
                store_block() {
        assert(alloc_size >=4); // "Allocation size must be atleast 4"
        assert(alloc_size == 4 || alloc_size % 4 == 0); //"Allocation size must be aligned to 4"

        pthread_mutex_init(&lock, nullptr);
    }

    uint8_t *get_memory();

    void free(const uint8_t *pheader);

    constexpr auto get_alloc_size() const { return alloc_size; }
}; // class fixed_memory

class memory {
public:
    static constexpr size_t max_allocation_size = 1024;
    static constexpr size_t max_chunk = max_allocation_size >> 2;

private:
    fixed_memory mem_array[max_chunk];
    
    inline uint8_t *get_memory(const size_t alloc_size) {
        const size_t chunk_index = (alloc_size >> 2) - 1;

        auto &memstore = mem_array[chunk_index];

        if constexpr (config::debug) {
            assert(memstore.get_alloc_size() == alloc_size + sizeof(fixed_memory_alloc_info));
        }

        uint8_t *memptr = memstore.get_memory();
        fixed_memory_alloc_info *pallocinfo = (fixed_memory_alloc_info *)memptr;
        pallocinfo->alloc_size = alloc_size;
        return memptr + sizeof(fixed_memory_alloc_info);
    }

public:
    memory();

    template <typename T, typename... ARGS>
    inline T *alloc(ARGS&... args) {
        constexpr size_t alloc_size = (sizeof(T) + 3) & (~3);
        static_assert(alloc_size <= max_allocation_size, "This allocator support maximum max_allocation_size memory");

        return new (get_memory(alloc_size)) T(args...);;
    }

    inline void *alloc(const size_t alloc_size) {
        const size_t new_alloc_size = (alloc_size + 3) & (~3);
        return (void *)get_memory(new_alloc_size);
    }

    // Passing base pointer will cause trouble
    // Do not pass base pointer
    template <typename T>
    inline void free(T *value) {
        uint8_t *pheader = (uint8_t *)value - sizeof(fixed_memory_alloc_info);
        fixed_memory_alloc_info *pmeminfo = (fixed_memory_alloc_info *)pheader;
        const size_t chunk_index = (pmeminfo->alloc_size >> 2) - 1;
        mem_array[chunk_index].free(pheader);
    }

    template <typename T>
    inline void free_debug(T *value, const size_t alloc_size) {
        const size_t new_alloc_size = (alloc_size + 3) & (~3);
        uint8_t *pheader = (uint8_t *)value - sizeof(fixed_memory_alloc_info);
        fixed_memory_alloc_info *pmeminfo = (fixed_memory_alloc_info *)pheader;
        assert(new_alloc_size == pmeminfo->alloc_size);
        const size_t chunk_index = (pmeminfo->alloc_size >> 2) - 1;
        mem_array[chunk_index].free(pheader);
    }


}; // class memory

extern memory allocator;

} // namespace rohit