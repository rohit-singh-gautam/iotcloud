////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/memory.hh>

namespace rohit {

memory allocator;

uint8_t *fixed_memory::get_memory() {
    uint8_t *memptr;
    if (free_start_index == null_index) {
        // Memory is full now require to make bigger allocation

        // Adding current store to final list
        // Adding first store as zero store save one comparison at each malloc
        // Hence we will store to free list only if current_store capacity is
        // not zero
        if (current_store->capacity != 0) store_to_free[to_free_count++] = current_store;
        // Re initialing current store with double memory capacity
        used_index = 0;
        auto new_capacity = current_store->capacity * 2;
        current_store = new memory_store(alloc_size, new_capacity);

        // Increase free index list
        auto free_linked_index_temp = free_linked_index;
        free_linked_index = new uint32_t[new_capacity];

        // Initializing free list, keeping zero free for current allocation
        free_start_index = 1;
        uint32_t free_index = 1, next_index = 2;
        while (free_index < new_capacity - 1) {
            free_linked_index[free_index] = next_index;
            free_index = next_index;
            ++next_index;
        }
        free_linked_index[free_index] = null_index;

        delete[] free_linked_index_temp;

        // Allocating memory at zero index
        memptr = current_store->store;
        ++current_store->count;
    } else {
        // allocate memory from free list
        auto alloc_index = free_start_index;
        free_start_index = free_linked_index[alloc_index];

        memptr = current_store->store + alloc_size * alloc_index;
        ++current_store->count;
    }

    return memptr;

} // uint8_t *fixed_memory::get_memory

void fixed_memory::free(const void *pvalue) {
    if (to_free_count) {
        for (size_t index; index < to_free_count; ++index) {
            memory_store *temp_store = store_to_free[index];
            if (temp_store->final_free(pvalue, alloc_size)) {
                // Memory is from this store
                if (temp_store->count == 0) {
                    // All freed release this store;
                    delete temp_store;
                    --to_free_count;
                    for(; index < to_free_count; ++index) {
                        store_to_free[index] = store_to_free[index + 1];
                    }
                }

                // Memory freed hence returning
                return;
            }
        }
    }

    // Throw error is memory is not allocated from current store
    assert(current_store->in_store(pvalue, alloc_size));

    // if current store is empty throw error
    assert(current_store->count);

    uint32_t mem_index = (uint8_t *)pvalue - current_store->store;
    free_linked_index[mem_index] = free_start_index;
    free_start_index = mem_index;
    --current_store->count;
} // void fixed_memory::free

}