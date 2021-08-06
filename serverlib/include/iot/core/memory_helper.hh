////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/config.hh>
#include <iot/core/types.hh>
#include <iot/core/error.hh>


namespace rohit {

struct mem {
    void    *ptr;
    size_t  size;
    mem(const size_t size) : size(size) { ptr = (void *)malloc(size); }
    constexpr mem(void *ptr = nullptr, size_t size = 0) : ptr(ptr), size(size) {}
    constexpr mem(const mem &val) : ptr(val.ptr), size(val.size) {}
    constexpr mem(const mem &&val) : ptr(val.ptr), size(val.size) {
        ptr = nullptr;
        if constexpr(config::debug) {
            size = 0; // Avoiding this for optimization
        }
        // It is expected that caller must not use move after move
    }

    constexpr void *operator=(void *rhs) {
        if constexpr (config::debug) {
            if (ptr != nullptr) {
                throw exception_t(err_t::CRYPTO_MEMORY_BAD_ASSIGNMENT);
            }
        }
        return ptr = rhs;
    }
    constexpr bool operator==(void *rhs) const { return ptr == rhs; }
    constexpr operator void *() const { return ptr; }
    constexpr operator uint8_t *() const { return (uint8_t *)ptr; }

    constexpr uint8_t *begin() { return (uint8_t *)ptr; }
    constexpr uint8_t *end() { return (uint8_t *)ptr + size; }
    constexpr const uint8_t *begin() const { return (uint8_t *)ptr; }
    constexpr const uint8_t *end() const { return (uint8_t *)ptr + size; }
};

std::ostream& operator<<(std::ostream& os, const mem &binary);

struct malloc_mem : public mem {
    using mem::mem;

    inline ~malloc_mem() { 
        if (ptr != nullptr) free(ptr);
    }
};

} // namespace rohit