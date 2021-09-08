////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

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