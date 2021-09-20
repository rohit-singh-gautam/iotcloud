////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/ipv6addr.hh>

namespace rohit {

char *http_add_404_Not_Found(
            char *const buffer,
            const ipv6_socket_addr_t &local_address,
            const char *date_str,
            const size_t date_str_size);

char *http_add_400_Bad_Request(
            char *const buffer,
            const ipv6_socket_addr_t &local_address,
            const char *date_str,
            const size_t date_str_size);

char *http_add_405_Method_Not_Allowed(
            char *const buffer,
            const ipv6_socket_addr_t &local_address,
            const char *date_str,
            const size_t date_str_size);

char *http_add_505_HTTP_Version_Not_Supported(
            char *const buffer,
            const ipv6_socket_addr_t &local_address,
            const char *date_str,
            const size_t date_str_size);

char *http_add_400_Bad_Request_HTTP2(
            char *const buffer,
            const ipv6_socket_addr_t &local_address,
            const char *date_str,
            const size_t date_str_size);

} // namespace rohit