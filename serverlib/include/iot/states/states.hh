#pragma once

#include <core/types.hh>
#include "statesentry.hh"
#include <assert.h>

namespace rohit {

enum class state_t : state_type {
#define STATE_ENTRY(x, y) x,
        STATE_ENTRY_LIST
#undef STATE_ENTRY
};

template <state_t state> struct state_description { };
#define STATE_ENTRY(x, y) template <> struct state_description<state_t::x> { \
        static constexpr const char id[] = #x; \
        static constexpr const size_t id_size = sizeof(#x); \
        static constexpr const char description[] = #y; \
        static constexpr const size_t description_size = sizeof(#y); \
    };
    STATE_ENTRY_LIST
#undef STATE_ENTRY


struct state_c {
    const state_t id;
    state_c(const state_t id) : id(id) { }

    constexpr const char *to_string() {
        switch (id) {
        default: // This will avoid error, such condition will never reach
            assert(true);
#define STATE_ENTRY(x, y) case state_t::x: return #y;
    STATE_ENTRY_LIST
#undef STATE_ENTRY
        }
    }

    constexpr const char*() { return to_string(); }
};



} // namespace rohit
