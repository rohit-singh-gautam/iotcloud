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

#include "types.hh"
#include "math.hh"
#include <algorithm>
#include <ostream>

namespace rohit {

template <number_case number_case = number_case::lower>
constexpr void guid_t_str_copy_helper(char * const dest, size_t &dest_i, const guid_t &guid, size_t &guid_i) {
    constexpr const char (&hex)[sizeof(lower_case_numbers)]
        = number_case == number_case::lower ? lower_case_numbers : upper_case_numbers;
    dest[dest_i++] = hex[guid[guid_i] >> 4];
    dest[dest_i++] = hex[guid[guid_i++] & 0x0f];
}

template <number_case number_case = number_case::lower, bool null_terminated = true>
constexpr size_t to_string(const guid_t &val, char * const dest_ptr) {
    size_t dest_i = 0;
    size_t guid_i = 0;
    for(;guid_i < 4;) {
        guid_t_str_copy_helper<number_case>(dest_ptr, dest_i, val, guid_i);
    }
    dest_ptr[dest_i++] = '-';

    for(;guid_i < 6;) {
        guid_t_str_copy_helper<number_case>(dest_ptr, dest_i, val, guid_i);
    }
    dest_ptr[dest_i++] = '-';

    for(;guid_i < 8;) {
        guid_t_str_copy_helper<number_case>(dest_ptr, dest_i, val, guid_i);
    }
    dest_ptr[dest_i++] = '-';

    for(;guid_i < 10;) {
        guid_t_str_copy_helper<number_case>(dest_ptr, dest_i, val, guid_i);
    }
    dest_ptr[dest_i++] = '-';

    for(;guid_i < 16;) {
        guid_t_str_copy_helper<number_case>(dest_ptr, dest_i, val, guid_i);
    }

    if constexpr (null_terminated == true) {
        dest_ptr[dest_i++] = '\0';
    }
    return dest_i;
}

constexpr void to_guid_helper(const char * const src, size_t &src_i, uint8_t * const guid_8, size_t &guid_8_i) {
    guid_8[guid_8_i++] = char_to_int[(size_t)src[src_i]] * 16 + char_to_int[(size_t)src[src_i+1]];
    src_i += 2;
}

constexpr guid_t to_guid(const char * const guidstr_ptr, size_t * const plen = nullptr) {
    uint8_t guid_8[guid_t::size] = {};
    size_t guidstr_i = 0;
    size_t guid_i = 0;
    // example f81d4fae-7dec-11d0-a765-00a0c91e6bf6
    for(;guid_i < 4;) {
        to_guid_helper(guidstr_ptr, guidstr_i, guid_8, guid_i);
    }
    ++guidstr_i;

    for(;guid_i < 6;) {
        to_guid_helper(guidstr_ptr, guidstr_i, guid_8, guid_i);
    }
    ++guidstr_i;

    for(;guid_i < 8;) {
        to_guid_helper(guidstr_ptr, guidstr_i, guid_8, guid_i);
    }
    ++guidstr_i;

    for(;guid_i < 10;) {
        to_guid_helper(guidstr_ptr, guidstr_i, guid_8, guid_i);
    }
    ++guidstr_i;

    for(;guid_i < 16;) {
        to_guid_helper(guidstr_ptr, guidstr_i, guid_8, guid_i);
    }

    if (plen != nullptr) *plen = guidstr_i;
    return guid_t(guid_8);
}

inline std::ostream& operator<<(std::ostream& os, const guid_t &guid) {
    char str[guid_t::guid_string_withnull_size] = {};
    to_string(guid, str);
    return os << str;
}

} // namespace rohit {