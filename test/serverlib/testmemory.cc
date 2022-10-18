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
#include <iostream>
#include <stack>

void one_alloc_multiple_times(const int count, const size_t size) {
    std::cout << "Test one_alloc_multiple_times size " << size << std::endl;
    void **mem_arr = (void **)malloc(count * sizeof(void *));
    for(int index = 0; index < count; ++index) {
        mem_arr[index] = rohit::allocator.alloc(size);
        uint8_t *pmem = (uint8_t *)mem_arr[index];
        pmem[0] = '\0';
        pmem[1] = '\0';
        pmem[2] = '\0';
        pmem[3] = '\0';
        pmem[size - 1] = '\0';
        pmem[size - 2] = '\0';
        pmem[size - 3] = '\0';
        pmem[size - 4] = '\0';
    }

    for(int index = 0; index < count; ++index) {
        uint8_t *pmem = (uint8_t *)mem_arr[index];
        rohit::allocator.free_debug(pmem, size);
    }

    free(mem_arr);
}

void zigsaw_multiple_times(const int start, const int end, const int step, const size_t size) {
    std::cout << "Test zigsaw_multiple_times size " << size << std::endl;
    std::stack<void *> mem;
    bool alloc = true;
    void *pmem;
    for(int count = end; count > start; count -= step) {
        for(int index = 0; index < count; ++index) {
            if (alloc) {
                pmem = rohit::allocator.alloc(size);
                mem.push(pmem);
            } else {
                pmem = mem.top();
                mem.pop();
                rohit::allocator.free(pmem);
            }
            alloc = !alloc;
        }
    }

    while(!mem.empty()) {
        pmem = mem.top();
        mem.pop();
        rohit::allocator.free(pmem);
    }
}

int main() {
    std::cout << "sizeof(fixed_memory_alloc_info) = " << sizeof(rohit::fixed_memory_alloc_info) << std::endl;
    std::cout << "sizeof(fixed_memory_free_info) = " << sizeof(rohit::fixed_memory_free_info) << std::endl;
    one_alloc_multiple_times(10000, 4);
    one_alloc_multiple_times(10000, 4);
    one_alloc_multiple_times(10000, 4);
    one_alloc_multiple_times(10000, 4);
    one_alloc_multiple_times(10000, 25);
    one_alloc_multiple_times(10000, 25);
    one_alloc_multiple_times(10000, 25);
    one_alloc_multiple_times(10000, 25);
    one_alloc_multiple_times(10000, 102);
    one_alloc_multiple_times(10000, 102);
    one_alloc_multiple_times(10000, 102);
    one_alloc_multiple_times(10000, 102);
    one_alloc_multiple_times(10000, 1024);
    one_alloc_multiple_times(10000, 1024);
    one_alloc_multiple_times(10000, 1024);
    one_alloc_multiple_times(10000, 1024);

    zigsaw_multiple_times(9863, 589332, 731, 4);
    zigsaw_multiple_times(1000, 10000, 1000, 25);
    zigsaw_multiple_times(1000, 10000, 1000, 102);
    zigsaw_multiple_times(1000, 10000, 1000, 1024);
    return 0;
}
