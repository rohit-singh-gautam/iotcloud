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
#define STATE_ENTRY(x, y) case state_t::x: return os << #x " " y;
        STATE_ENTRY_LIST
#undef STATE_ENTRY
    }
}


} // namespace rohit
