////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/types.hh>
#include <iot/core/math.hh>
#include <iot/core/guid.hh>
#include <iot/core/ipv6addr.hh>
#include <iot/core/varadic.hh>

namespace rohit {

constexpr bool to_bool(const char *value) {
    return *value == 'T' || *value == 't';
}

constexpr bool to_type(const type_identifier id, void *store, const char *value) {
    switch (id) {
    case type_identifier::char_t: 
        *(char_t *)store = *value;
        break;
    
    case type_identifier::bool_t:
        *(bool_t *)store = to_bool(value);
        break;
    
    case type_identifier::uint8_t:
        *(uint8_t *)store = to_uint<uint8_t>(value);
        break;

    case type_identifier::uint16_t:
        *(uint16_t *)store = to_uint<uint16_t>(value);
        break;

    case type_identifier::uint32_t:
        *(uint32_t *)store = to_uint<uint32_t>(value);
        break;

    case type_identifier::uint64_t:
        *(uint64_t *)store = to_uint<uint64_t>(value);
        break;

    case type_identifier::string_t:
        *(const char **)store = value;
        break;

    case type_identifier::guid_t:
        *(guid_t *)store = to_guid(value);
        break;

    case type_identifier::ipv6_addr_t:
        *(ipv6_addr_t *)store = to_ipv6_addr_t(value);
        break;

    case type_identifier::ipv6_port_t: {
            auto ipv6_port_val = to_ipv6_port_t(value);
            copyvaradic((uint8_t *)store, ipv6_port_val);
            break;
        }

    case type_identifier::ipv6_socket_addr_t: {
            auto ipv6_socket_val = to_ipv6_socket_addr_t(value);
            copyvaradic((uint8_t *)store, ipv6_socket_val);
            break;
        }
        
    default:
        return false;

    }

    return true;
}


}