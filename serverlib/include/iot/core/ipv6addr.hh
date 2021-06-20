////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include "types.hh"
#include "assert.h"
#include <iot/core/math.hh>
#include <ostream>

namespace rohit {

constexpr size_t ipv6_addr_t_max_hex_string = 40; // "xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx"

// This function parse ipv6 string to ipv6_addr_t
// We are iterating twice on string to aviod dual copy
// First iteration will find position of comacting '::'
// Second iteration will file the ipv6_addr_t
// TODO: Error handling
template <bool bracesEnd = false>
constexpr const ipv6_addr_t to_ipv6_addr_t(const char *ipv6str, size_t *len = nullptr) {
    constexpr size_t no_compat = 8;
    size_t compacted_pos_start  = no_compat;
    ipv6_addr_t addr = {};

    const char *ipv6iter = ipv6str;
    char current = 0;
    bool lastcolon = false;
    size_t pos = 0;
    while((current = *ipv6iter++)) {
        if constexpr (bracesEnd) if (current == ']') break;
        switch(current) {
        case ':':
            if (lastcolon) {
                compacted_pos_start = pos;
            }
            lastcolon = true;
            pos++;
            break;
        default:
            lastcolon = false;
            break;
        }
    };

    size_t compacted_pos_skip = ipv6_addr16_size - pos;
    pos = 0;
    lastcolon = false;
    ipv6iter = ipv6str;
    uint16_t hexval = 0;
    while((current = *ipv6iter++)) {
        if constexpr (bracesEnd) if (current == ']') break;
        switch(current) {
        case ':':
            addr.addr_16[pos] = changeEndian(hexval);
            if (pos == compacted_pos_start) {
                pos += compacted_pos_skip;
            } else {
                pos++;
                hexval = 0;
            }
            break;
        default:
            hexval = hexval*16 + char_to_int[(size_t)current]; 
            lastcolon = false;
            break;
        }
    };
    assert(pos == 7);
    addr.addr_16[pos] = changeEndian(hexval);

    if (len != nullptr) *len = (size_t)(ipv6iter - ipv6str);

    return addr;
}

template <number_case number_case = number_case::lower, bool null_terminated = true>
constexpr size_t to_string(const ipv6_addr_t &val, char *dest) {
    constexpr size_t radix = 16;
    constexpr size_t no_compat = 8;
    size_t compact_start = no_compat;
    size_t compact_size = 1;

    size_t current_size = 0;
    for(size_t index = 0; index < 8; ++index) {
        if (!val.addr_16[index]) {
            if (++current_size > compact_size) {
                compact_size = current_size;
                compact_start = index + 1 - compact_size;
            }
        }
        else current_size = 0;
    }

    char *pstr = dest;
    size_t index = 0;
    while(index < 8) {
        if (index == compact_start) {
            if (index == 0) *pstr++ = ':';
            *pstr++ = ':';

            index += compact_size;
        } else {
            auto addr_segment = changeEndian(val.addr_16[index]);
            size_t str_size = to_string<uint16_t, radix, number_case, false>(addr_segment, pstr);
            pstr += str_size;
            if (index != 7) {
                *pstr++ = ':';
            }
            ++index;
        }
    }

    if constexpr (null_terminated == true) {
        *pstr++ = '\0';
    }
    return (size_t)(pstr - dest);
}

constexpr const ipv6_socket_addr_t string_to_ipv6_socket_addr_t(const char *ipv6str, size_t *plen = nullptr) {
    const char *pipv6str = ipv6str + 1; // Skipping opening bracket
    size_t len = 0;

    auto addr = to_ipv6_addr_t<true>(pipv6str, &len);
    pipv6str += len;
    
    ++pipv6str; // Skipping ':'

    ipv6_port_t port = to_uint<uint16_t>(pipv6str, &len);
    pipv6str += len;

    if (plen != nullptr) *plen = (size_t)(pipv6str - ipv6str);
    return ipv6_socket_addr_t(addr, port);
}

// "[xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx]:65535"
constexpr size_t ipv6_socket_addr_t_max_hex_string = ipv6_addr_t_max_hex_string + 8;

template <number_case number_case = number_case::lower, bool null_terminated = true>
constexpr size_t to_string(const ipv6_socket_addr_t &val, char *dest) {
    char *pstr = dest;
    *pstr++ = '[';
    pstr += to_string<number_case, false>(val.addr, pstr);
    *pstr++ = ']';
    *pstr++ = ':';
    pstr += to_string<uint16_t, 10, number_case, null_terminated>(val.port, pstr);
    return (size_t)(pstr - dest);
}

class ipv6_addr_c {
private:
    const ipv6_addr_t addr;

public:
    constexpr ipv6_addr_c(const ipv6_addr_c &value) : addr(value.addr) {}
    constexpr ipv6_addr_c(const char *value) : addr(to_ipv6_addr_t(value)) {}

    constexpr operator ipv6_addr_t() const { return addr; }

};

inline std::ostream& operator<<(std::ostream& os, const ipv6_addr_t &ipv6addr) {
    char str[ipv6_addr_t_max_hex_string] = {};
    to_string(ipv6addr, str);
    return os << str;
}

inline std::ostream& operator<<(std::ostream& os, const ipv6_socket_addr_t &ipv6sockaddr) {
    char str[ipv6_socket_addr_t_max_hex_string] = {};
    to_string(ipv6sockaddr, str);
    return os << str;
}


constexpr ipv6_socket_addr_t::ipv6_socket_addr_t(const char *addrstr, const ipv6_port_t port)
    : addr(to_ipv6_addr_t(addrstr)), port(port) { }


} // namespace rohit