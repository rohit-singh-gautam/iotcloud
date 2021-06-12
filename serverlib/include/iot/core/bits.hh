////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include "types.hh"

namespace rohit {

constexpr size_t bits_to_uint64_index(const size_t value) {
    return value >> 6;
}

constexpr size_t bits_to_uint64_map(const size_t value) {
    return (value & 0x3fu);
}

} // namespace rohit