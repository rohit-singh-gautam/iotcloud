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

    case type_identifier::int8_t:
        *(int8_t *)store = to_int<int8_t>(value);
        break;

    case type_identifier::int16_t:
        *(int16_t *)store = to_int<int16_t>(value);
        break;

    case type_identifier::int32_t:
        *(int32_t *)store = to_int<int32_t>(value);
        break;

    case type_identifier::int64_t:
        *(int64_t *)store = to_int<int64_t>(value);
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