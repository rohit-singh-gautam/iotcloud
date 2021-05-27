// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in)

#pragma once

#include <string>
#include <istream>

#include "http11.tab.hh"
#include "http11scanner.hh"
#include "http11.hh"

namespace iotcloud {

class http11driver {
public:
    ~http11driver();

    void parse(std::string &text);

    http_header_request header;

private:
    void parse_internal(std::istream &iss);

    iotcloud::parser *parser  = nullptr;
    iotcloud::http11scanner *scanner = nullptr;

public:
    friend std::ostream& operator<<(std::ostream& os, const http11driver& driver);
};

inline std::ostream& operator<<(std::ostream& os, const http11driver& driver) { return os << driver.header; }

inline const char *skipFirstAndSpace(const char *str) {
    ++str;
    while(*str && (*str == ' ' || *str == '\t')) ++str;

    return str;
}

}