////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/ipv6addr.hh>

namespace rohit {

char *http_add_404_Not_Found(char *const buffer, const ipv6_socket_addr_t &local_address);

} // namespace rohit