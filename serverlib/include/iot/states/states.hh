////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/types.hh>
#include <assert.h>
#include <unordered_map>

namespace rohit {

template <state_t state> struct state_description { };
#define STATE_ENTRY(x, y) template <> struct state_description<state_t::x> { \
        static constexpr const char id[] = #x; \
        static constexpr const size_t id_size = sizeof(#x); \
        static constexpr const char description[] = y; \
        static constexpr const size_t description_size = sizeof(y); \
    };
    STATE_ENTRY_LIST
#undef STATE_ENTRY


template <bool null_terminated = true>
constexpr size_t to_string(const state_t &val, char *dest) {
    switch (val) {
    default: // This will avoid error, such condition will never reach
        assert(true);
#define STATE_ENTRY(x, y) \
    case state_t::x: { \
        constexpr size_t desc_size = sizeof(y) \
                - (null_terminated ? 0: 1); \
        constexpr const char desc[] = y; \
        std::copy(desc, desc + desc_size, dest); \
        return desc_size; }
        STATE_ENTRY_LIST
#undef STATE_ENTRY
    }
}

inline std::ostream& operator<<(std::ostream& os, const state_t &state) {
    switch (state) {
    default: // This will avoid error, such condition will never reach
        assert(true);
#define STATE_ENTRY(x, y) case state_t::x: return os << y;
        STATE_ENTRY_LIST
#undef STATE_ENTRY
    }
}


} // namespace rohit
