#include "serversocket.hh"
#include "iotcloudmath.hh"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>

namespace iotcloud {

const std::string ipv6_addr::to_string() const { 
    // Stored memory for string like "000.000.000.000:00000"
    static const size_t displayString_size = INET6_ADDRSTRLEN + 6;
    const std::string displayString(displayString_size, '\0');

    char *ipv6_str = const_cast<char *>(displayString.c_str());
    inet_ntop(AF_INET6, &addr.sin6_addr, ipv6_str, displayString_size);

    char *pStr = ipv6_str;
    while(*pStr) ++pStr;

    *pStr++ = ':';
    math::integerToString(pStr, getPort());
    return displayString;
}

} // namespace iotcloud