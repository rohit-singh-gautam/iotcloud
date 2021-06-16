////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/memory.hh>

void one_alloc_multiple_times(const int count, const size_t size) {
    void *mem_arr[count];
    for(int index = 0; index < count; ++index) {
        mem_arr[index] = rohit::allocator.alloc(size);
    }

    for(int index = 0; index < count; ++index) {
        rohit::allocator.free(mem_arr[index]);
    }
}

int main() {
    one_alloc_multiple_times(1000000, 25);
    one_alloc_multiple_times(100000, 102);
    return 0;
}
