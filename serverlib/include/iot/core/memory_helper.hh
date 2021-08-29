////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/config.hh>
#include <iot/core/types.hh>
#include <iot/core/error.hh>
#include <iot/core/math.hh>

namespace rohit {

template <typename T = void>
struct mem {
    using itr_type = std::conditional_t< std::is_same<T, void>::value, uint8_t, T>;
    T    *ptr;
    size_t  size; // Size in bytes
    mem(const size_t size) : size(size) { ptr = (T *)malloc(size); }

    template <typename inT = T>
    constexpr mem(inT *ptr = nullptr, size_t size = 0) : ptr((T *)ptr), size(size) {}

    constexpr mem(const mem<T> &val) : ptr(val.ptr), size(val.size) {}

    constexpr mem(mem<T> &&val) : ptr(val.ptr), size(val.size) {
        val.ptr = nullptr;
        if constexpr(config::debug) {
            val.size = 0; // Avoiding this for optimization
        }
        // It is expected that caller must not use move after move
    }

    template <typename inT>
    constexpr T *operator=(inT *rhs) {
        return ptr = (T *)rhs;
    }

    constexpr bool operator==(void *rhs) const { return ptr == rhs; }

    template <typename inT>
    constexpr bool operator==(inT *rhs) const { return ptr == (T *)rhs; }

    template <typename inT>
    constexpr operator inT *() const { return (inT *)ptr; }

    constexpr itr_type *begin() { return (T *)ptr; }
    constexpr itr_type *end() { return (T *)ptr + size; }
    constexpr const itr_type *begin() const { return (T *)ptr; }
    constexpr const itr_type *end() const { return (T *)ptr + size; }
};

template <typename T = void>
std::ostream& operator<<(std::ostream& os, const mem<T> &binary) {
    uint8_t *v_ptr_begin = (uint8_t *)binary.ptr;
    uint8_t *v_ptr_end = (uint8_t *)binary.ptr + binary.size;
    for(; v_ptr_begin != v_ptr_end; ++v_ptr_begin) {
        uint8_t value = *v_ptr_begin;
        os << upper_case_numbers[value/16] << upper_case_numbers[value%16];
    }
    return os;
}

template <typename T = void>
struct malloc_mem : public mem<T> {
    using mem<T>::ptr;
    using mem<T>::size;

    using mem<T>::mem;

    inline ~malloc_mem() { 
        if (ptr != nullptr) free(ptr);
    }
};

template <typename T = void>
struct mem_new : public mem<T> {
    using mem<T>::ptr;
    using mem<T>::size;

    using mem<T>::mem;

    inline ~mem_new() { 
        if (ptr != nullptr) delete[] ptr;
    }
};

} // namespace rohit