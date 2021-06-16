////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/memory.hh>
#include <memory.h>

namespace rohit {

memory allocator;

memory::memory() : mem_array {
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

uint8_t *fixed_memory::get_memory() {
    uint8_t *memptr;
    pthread_mutex_lock(&lock);
    if (free_start_index == null_index) {
        // Memory is full now require to make bigger allocation

        current_capacity *= 2;
        auto memory_size = current_capacity * alloc_size;
        ++last_store_index;

        if constexpr (config::debug) {
            assert(memory_size <= null_index.memory_index + 1);
        }

        store_block[last_store_index] = (uint8_t *)malloc(memory_size);

        // Initializing free list, keeping zero free for current allocation
        free_start_index = {static_cast<uint8_t>(last_store_index), alloc_size};
        uint32_t free_index = alloc_size;
        uint32_t next_index = free_index + alloc_size;
        auto memory_size_one_less = memory_size - alloc_size;
        uint8_t *current_store_block = store_block[last_store_index];
        while (free_index < memory_size_one_less) {
            *(fixed_memory_free_info*)(current_store_block + free_index) = 
                {static_cast<uint8_t>(last_store_index), next_index};
            free_index = next_index;
            next_index += alloc_size;
        }
        *(fixed_memory_free_info*)(current_store_block + free_index) = null_index;

        // Allocating memory at zero index
        memptr = current_store_block;
        fixed_memory_alloc_info *pallocinfo = (fixed_memory_alloc_info *)memptr;
        pallocinfo->store_index = last_store_index;
        // current_store_index can be changed from another thread
        pthread_mutex_unlock(&lock);
    } else {
        // allocate memory from free list
        uint32_t store_index = free_start_index.store_index;
        memptr = store_block[store_index] + free_start_index.memory_index;
        free_start_index = *(fixed_memory_free_info*)memptr;

        pthread_mutex_unlock(&lock);
        fixed_memory_alloc_info *pallocinfo = (fixed_memory_alloc_info *)memptr;
        pallocinfo->store_index = store_index;
    }

    if constexpr (config::debug) {
        fixed_memory_alloc_info *pallocinfo = (fixed_memory_alloc_info *)memptr;
        pallocinfo->memory_check = fixed_memory_alloc_info::default_memory_check;
    }

    return memptr;

} // uint8_t *fixed_memory::get_memory

void fixed_memory::free(const uint8_t *pheader) {
    pthread_mutex_lock(&lock);

    fixed_memory_alloc_info alloc_info = *(fixed_memory_alloc_info *)pheader;
    *(fixed_memory_free_info *)pheader = free_start_index;

    uint8_t *current_store_block = store_block[alloc_info.store_index];
    free_start_index.memory_index = pheader - current_store_block;
    free_start_index.store_index = alloc_info.store_index;

    if constexpr (config::debug) {
        assert((free_start_index.memory_index % alloc_size) == 0);
        assert(alloc_info.memory_check == fixed_memory_alloc_info::default_memory_check );
    }

    pthread_mutex_unlock(&lock);
} // void fixed_memory::free

}