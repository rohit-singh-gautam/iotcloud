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

#include <iostream>
#include <string>

#include <iot/core/error.hh>
#include <http11parser.hh>
#include <http11scanner.hh>
#include <http11.hh>

namespace rohit {

class http11driver {
public:
    ~http11driver();

    err_t parse(std::string &text);

    http_header_request header;

private:
    err_t parse_internal(std::istream &iss);

    rohit::parser *parser  = nullptr;
    rohit::http11scanner *scanner = nullptr;

public:
    friend std::ostream& operator<<(std::ostream& os, const http11driver& driver);
};

inline std::ostream& operator<<(std::ostream& os, const http11driver& driver) { return os << driver.header; }

inline const char *skipFirstAndSpace(const char *str) {
    ++str;
    while(*str && (*str == ' ' || *str == '\t')) ++str;

    return str;
}

} // namespace rohit