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

#include <iot/core/ipv6addr.hh>
#include <iot/core/math.hh>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstring>

uint16_t success = 0;
uint16_t failure = 0;

int main() {
    const char *addrstrlist[] = {
        "2a::2e",
        "::1",
        "::",
        "1a:bc:ef::4e",
        "aa:bc::",
        "AA:cB::F9"
    };
    for(const char *addrstr: addrstrlist) {
        rohit::ipv6_addr_t addr = rohit::to_ipv6_addr_t(addrstr);
        rohit::ipv6_addr_t addr2;
        inet_pton(AF_INET6, addrstr, &addr2);
        std::cout << "Address: " << addr << ", inet_pton: " << addr2 << std::endl;

        if (memcmp(&addr, &addr2, sizeof(rohit::ipv6_addr_t)) == 0) ++success;
        else ++failure;
    }

    const char *sockstrlist[] = {
        "[::1]:65535",
        "[2a::2e]:8080",
        "[::]:0",
        "[1a:bc:ef::4e]:33",
        "[aa:bc::]:0022",
        "[AA:cB::F9]:4533"
    };
    for(const char *addrstr: sockstrlist) {
        rohit::ipv6_socket_addr_t addr = rohit::to_ipv6_socket_addr_t(addrstr);
        std::cout << "Address: " << addr << std::endl;
    }

    std::cout << "Testing ipv6_port_t" << std::endl;
    rohit::ipv6_port_t port(8080); std::cout << "Simple uint16_t: " << port << std::endl;
    rohit::ipv6_port_t port1(port); std::cout << "Simple ipvt_port_t: " << port << std::endl;
    std::cout << std::endl;

    std::cout << "Testing ipv6_socket_addr_t" << std::endl;
    rohit::ipv6_socket_addr_t addr("::1", 8080); std::cout << "Simple ipv6_socket_addr_t: " << addr << std::endl;
    std::cout << std::endl;

    std::cout << "Summary: success(" << success << "), failure(" << failure << ")" << std::endl;

    return 0;
}