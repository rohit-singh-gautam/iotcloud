/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// This program is free software: you can redistribute it and/or modify it under the terms //
// of the GNU General Public License as published by the Free Software Foundation, either  //
// version 3 of the License, or (at your option) any later version.                        //
//                                                                                         //
// This program is distributed in the hope that it will be useful, but WITHOUT ANY         //
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A         //
// PARTICULAR PURPOSE. See the GNU General Public License for more details.                //
//                                                                                         //
// You should have received a copy of the GNU General Public License along with this       //
// program. If not, see <https://www.gnu.org/licenses/>.                                   //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <iot/core/memory.hh>
#include <memory.h>

namespace rohit {

memory allocator;

memory::memory() : mem_array {
    {   8}, {  16}, {  24}, {  32}, {  40}, {  48}, {  56}, {  64}, {  72}, {  80}, {  88}, {  96},
    { 104}, { 112}, { 120}, { 128}, { 136}, { 144}, { 152}, { 160}, { 168}, { 176}, { 184}, { 192},
    { 200}, { 208}, { 216}, { 224}, { 232}, { 240}, { 248}, { 256}, { 264}, { 272}, { 280}, { 288},
    { 296}, { 304}, { 312}, { 320}, { 328}, { 336}, { 344}, { 352}, { 360}, { 368}, { 376}, { 384},
    { 392}, { 400}, { 408}, { 416}, { 424}, { 432}, { 440}, { 448}, { 456}, { 464}, { 472}, { 480},
    { 488}, { 496}, { 504}, { 512}, { 520}, { 528}, { 536}, { 544}, { 552}, { 560}, { 568}, { 576},
    { 584}, { 592}, { 600}, { 608}, { 616}, { 624}, { 632}, { 640}, { 648}, { 656}, { 664}, { 672},
    { 680}, { 688}, { 696}, { 704}, { 712}, { 720}, { 728}, { 736}, { 744}, { 752}, { 760}, { 768},
    { 776}, { 784}, { 792}, { 800}, { 808}, { 816}, { 824}, { 832}, { 840}, { 848}, { 856}, { 864},
    { 872}, { 880}, { 888}, { 896}, { 904}, { 912}, { 920}, { 928}, { 936}, { 944}, { 952}, { 960},
    { 968}, { 976}, { 984}, { 992}, {1000}, {1008}, {1016}, {1024} } {}

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
        pallocinfo->memory_check_2 = fixed_memory_alloc_info::default_memory_check_2;
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
        assert(alloc_info.memory_check_2 == fixed_memory_alloc_info::default_memory_check_2 );
    }

    pthread_mutex_unlock(&lock);
} // void fixed_memory::free

}