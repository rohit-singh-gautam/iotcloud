////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once
#include <http11.hh>
#include <string>
#include <vector>

namespace rohit::http::v2 {

class dynamic_table_t {
    size_t max_size;

    std::vector<std::pair<http_header::FIELD, std::string>> entries;

public:
    dynamic_table_t(size_t max_size = 12) : max_size(max_size), entries(max_size) {}

    inline void update_size(size_t new_size) {
        max_size = new_size;
        if (new_size < entries.size()) {
            entries.resize(new_size);
        }
    }

    inline void insert(std::pair<http_header::FIELD, std::string> &entry) {
        if (entries.size() >= max_size) {
            // rfc7541 - 4.4.  Entry Eviction When Adding New Entries
            entries.clear();
        }
        entries.push_back(entry);
    }

    inline std::pair<http_header::FIELD, std::string> operator[](size_t index) {
        if (index >= entries.size()) {
            return {http_header::FIELD::IGNORE_THIS, ""};
        } else {
            return entries[index];
        }
    }

};

const std::pair<http_header::FIELD, std::string> static_table[] = {
    {http_header::FIELD::IGNORE_THIS, ""}, // 0 is not used
    {http_header::FIELD::Authority,""},
    {http_header::FIELD::Method, "GET"},
    {http_header::FIELD::Method, "POST"},
    {http_header::FIELD::Path, "/"},
    {http_header::FIELD::Path, "/index.html"},
    {http_header::FIELD::Scheme,  "http"},
    {http_header::FIELD::Scheme, "https"},
    {http_header::FIELD::Status, "200"},
    {http_header::FIELD::Status, "204"},
    {http_header::FIELD::Status, "206"}, // 10
    {http_header::FIELD::Status, "304"},
    {http_header::FIELD::Status, "400"},
    {http_header::FIELD::Status, "404"},
    {http_header::FIELD::Status, "500"},
    {http_header::FIELD::Accept_Charset, ""},
    {http_header::FIELD::Accept_Encoding, "gzip, deflate"},
    {http_header::FIELD::Accept_Language, ""},
    {http_header::FIELD::Accept_Ranges, ""},
    {http_header::FIELD::Accept, ""},
    {http_header::FIELD::Access_Control_Allow_Origin, ""}, // 20
    {http_header::FIELD::Age, ""},
    {http_header::FIELD::Allow, ""},
    {http_header::FIELD::Authorization, ""},
    {http_header::FIELD::Cache_Control, ""},
    {http_header::FIELD::Content_Disposition, ""},
    {http_header::FIELD::Content_Encoding, ""},
    {http_header::FIELD::Content_Language, ""},
    {http_header::FIELD::Content_Length, ""},
    {http_header::FIELD::Content_Location, ""},
    {http_header::FIELD::Content_Range, ""}, // 30
    {http_header::FIELD::Content_Type, ""},
    {http_header::FIELD::Cookie, ""},
    {http_header::FIELD::Date, ""},
    {http_header::FIELD::ETag, ""},
    {http_header::FIELD::Expect, ""},
    {http_header::FIELD::Expires, ""},
    {http_header::FIELD::From, ""},
    {http_header::FIELD::Host, ""},
    {http_header::FIELD::If_Match, ""},
    {http_header::FIELD::If_Modified_Since, ""}, // 40
    {http_header::FIELD::If_None_Match, ""},
    {http_header::FIELD::If_Range, ""},
    {http_header::FIELD::If_Unmodified_Since, ""},
    {http_header::FIELD::Last_Modified, ""},
    {http_header::FIELD::Link, ""},
    {http_header::FIELD::Location, ""},
    {http_header::FIELD::Max_Forwards, ""},
    {http_header::FIELD::Proxy_Authenticate, ""},
    {http_header::FIELD::Proxy_Authorization, ""},
    {http_header::FIELD::Range, ""}, // 50
    {http_header::FIELD::Referer, ""},
    {http_header::FIELD::Refresh, ""},
    {http_header::FIELD::Retry_After, ""},
    {http_header::FIELD::Server, ""},
    {http_header::FIELD::Set_Cookie, ""},
    {http_header::FIELD::Strict_Transport_Security, ""},
    {http_header::FIELD::Transfer_Encoding, ""},
    {http_header::FIELD::User_Agent, ""},
    {http_header::FIELD::Vary, ""},
    {http_header::FIELD::Via, ""}, // 60
    {http_header::FIELD::WWW_Authenticate, ""},
};

template <typename T, size_t N>
constexpr size_t count_static_array(const T (&)[N]) { return N; }

constexpr size_t static_table_len = count_static_array(static_table);

class node {
    int16_t symbol;

public:
    node *left;
    node *right;

    constexpr node() : symbol(-1), left(nullptr), right(nullptr) { }

    constexpr bool is_leaf() const { return left == nullptr && right == nullptr; }
    
    constexpr void set_leaf(const uint16_t symbol) {
        this->symbol = symbol;
    }

    constexpr bool is_eos() const { return symbol == 256; }
    constexpr char get_symbol() const { return (char) symbol; }
};

struct huffman_entry {
    const uint32_t code;
    const uint32_t code_len;

    constexpr huffman_entry(const uint32_t code, const uint32_t code_len) : code(code), code_len(code_len) { }
};

constexpr const huffman_entry static_huffman[] = {
    {    0x1ff8, 13}, {  0x7fffd8, 23}, { 0xfffffe2, 28}, { 0xfffffe3, 28}, { 0xfffffe4, 28}, { 0xfffffe5, 28}, { 0xfffffe5, 28}, { 0xfffffe7, 28}, //000-007
    { 0xfffffe8, 28}, {  0xffffea, 24}, {0x3ffffffc, 30}, { 0xfffffe9, 28}, { 0xfffffea, 28}, {0x3ffffffd, 30}, { 0xfffffeb, 28}, { 0xfffffec, 28}, //008-015
    { 0xfffffed, 28}, { 0xfffffee, 28}, { 0xfffffef, 28}, { 0xffffff0, 28}, { 0xffffff1, 28}, { 0xffffff2, 28}, {0x3ffffffe, 30}, { 0xffffff3, 28}, //016-023
    { 0xffffff4, 28}, { 0xffffff5, 28}, { 0xffffff6, 28}, { 0xffffff7, 28}, { 0xffffff8, 28}, { 0xffffff9, 28}, { 0xffffffa, 28}, { 0xffffffb, 28}, //024-031
    {      0x14,  6}, {     0x3f8, 10}, {     0x3f9, 10}, {     0xffa, 12}, {    0x1ff9, 13}, {      0x15,  6}, {      0xf8,  8}, {     0x7fa, 11}, //032-039
    {     0x3fa, 10}, {     0x3fb, 10}, {      0xf9,  8}, {     0x7fb, 11}, {      0xfa,  8}, {      0x16,  6}, {      0x17,  6}, {      0x18,  6}, //040-047
    {       0x0,  5}, {       0x1,  5}, {       0x2,  5}, {      0x19,  6}, {      0x1a,  6}, {      0x1b,  6}, {      0x1c,  6}, {      0x1d,  6}, //048-055
    {      0x1e,  6}, {      0x1f,  6}, {      0x5c,  7}, {      0xfb,  8}, {    0x7ffc, 15}, {      0x20,  6}, {     0xffb, 12}, {     0x3fc, 10}, //056-063
    {    0x1ffa, 13}, {      0x21,  6}, {      0x5d,  7}, {      0x5e,  7}, {      0x5f,  7}, {      0x60,  7}, {      0x61,  7}, {      0x62,  7}, //064-071
    {      0x63,  7}, {      0x64,  7}, {      0x65,  7}, {      0x66,  7}, {      0x67,  7}, {      0x68,  7}, {      0x69,  7}, {      0x6a,  7}, //072-079
    {      0x6b,  7}, {      0x6c,  7}, {      0x6d,  7}, {      0x6e,  7}, {      0x6f,  7}, {      0x70,  7}, {      0x71,  7}, {      0x72,  7}, //080-087
    {      0xfc,  8}, {      0x73,  7}, {      0xfd,  8}, {    0x1ffb, 13}, {   0x7fff0, 19}, {    0x1ffc, 13}, {    0x3ffc, 14}, {      0x22,  6}, //088-095
    {    0x7ffd, 15}, {       0x3,  5}, {      0x23,  6}, {       0x4,  5}, {      0x24,  6}, {       0x5,  5}, {      0x25,  6}, {      0x26,  6}, //096-103
    {      0x27,  6}, {       0x6,  5}, {      0x74,  7}, {      0x75,  7}, {      0x28,  6}, {      0x29,  6}, {      0x2a,  6}, {       0x7,  5}, //104-111
    {      0x2b,  6}, {      0x76,  7}, {      0x2c,  6}, {       0x8,  5}, {       0x9,  5}, {      0x2d,  6}, {      0x77,  7}, {      0x78,  7}, //112-119
    {      0x79,  7}, {      0x7a,  7}, {      0x7b,  7}, {    0x7ffe, 15}, {     0x7fc, 11}, {    0x3ffd, 14}, {    0x1ffd, 13}, { 0xffffffc, 28}, //120-127
    {   0xfffe6, 20}, {  0x3fffd2, 22}, {   0xfffe7, 20}, {   0xfffe8, 20}, {  0x3fffd3, 22}, {  0x3fffd4, 22}, {  0x3fffd5, 22}, {  0x7fffd9, 23}, //128-135
    {  0x3fffd6, 22}, {  0x7fffda, 23}, {  0x7fffdb, 23}, {  0x7fffdc, 23}, {  0x7fffdd, 23}, {  0x7fffde, 23}, {  0xffffeb, 24}, {  0x7fffdf, 23}, //136-143
    {  0xffffec, 24}, {  0xffffed, 24}, {  0x3fffd7, 22}, {  0x7fffe0, 23}, {  0xffffee, 24}, {  0x7fffe1, 23}, {  0x7fffe2, 23}, {  0x7fffe3, 23}, //144-151
    {  0x7fffe4, 23}, {  0x1fffdc, 21}, {  0x3fffd8, 22}, {  0x7fffe5, 23}, {  0x3fffd9, 22}, {  0x7fffe6, 23}, {  0x7fffe7, 23}, {  0xffffef, 24}, //152-159
    {  0x3fffda, 22}, {  0x1fffdd, 21}, {   0xfffe9, 20}, {  0x3fffdb, 22}, {  0x3fffdc, 22}, {  0x7fffe8, 23}, {  0x7fffe9, 23}, {  0x1fffde, 21}, //160-167
    {  0x7fffea, 23}, {  0x3fffdd, 22}, {  0x3fffde, 22}, {  0xfffff0, 24}, {  0x1fffdf, 21}, {  0x3fffdf, 22}, {  0x7fffeb, 23}, {  0x7fffec, 23}, //168-175
    {  0x1fffe0, 21}, {  0x1fffe1, 21}, {  0x3fffe0, 22}, {  0x1fffe2, 21}, {  0x7fffed, 23}, {  0x3fffe1, 22}, {  0x7fffee, 23}, {  0x7fffef, 23}, //176-183
    {   0xfffea, 20}, {  0x3fffe2, 22}, {  0x3fffe3, 22}, {  0x3fffe4, 22}, {  0x7ffff0, 23}, {  0x3fffe5, 22}, {  0x3fffe6, 22}, {  0x7ffff1, 23}, //184-191
    { 0x3ffffe0, 26}, { 0x3ffffe1, 26}, {   0xfffeb, 20}, {   0x7fff1, 19}, {  0x3fffe7, 22}, {  0x7ffff2, 23}, {  0x3fffe8, 22}, { 0x1ffffec, 25}, //192-199
    { 0x3ffffe2, 26}, { 0x3ffffe3, 26}, { 0x3ffffe4, 26}, { 0x7ffffde, 27}, { 0x7ffffdf, 27}, { 0x3ffffe5, 26}, {  0xfffff1, 24}, { 0x1ffffed, 25}, //200-207
    {   0x7fff2, 19}, {  0x1fffe3, 21}, { 0x3ffffe6, 26}, { 0x7ffffe0, 27}, { 0x7ffffe1, 27}, { 0x3ffffe7, 26}, { 0x7ffffe2, 27}, {  0xfffff2, 24}, //208-215
    {  0x1fffe4, 21}, {  0x1fffe5, 21}, { 0x3ffffe8, 26}, { 0x3ffffe9, 26}, { 0xffffffd, 28}, { 0x7ffffe3, 27}, { 0x7ffffe4, 27}, { 0x7ffffe5, 27}, //216-223
    {   0xfffec, 20}, {  0xfffff3, 24}, {   0xfffed, 20}, {  0x1fffe6, 21}, {  0x3fffe9, 22}, {  0x1fffe7, 21}, {  0x1fffe8, 21}, {  0x7ffff3, 23}, //224-231
    {  0x3fffea, 22}, {  0x3fffeb, 22}, { 0x1ffffee, 25}, { 0x1ffffef, 25}, {  0xfffff4, 24}, {  0xfffff5, 24}, { 0x3ffffea, 26}, {  0x7ffff4, 23}, //232-239
    { 0x3ffffeb, 26}, { 0x7ffffe6, 27}, { 0x3ffffec, 26}, { 0x3ffffed, 26}, { 0x7ffffe7, 27}, { 0x7ffffe8, 27}, { 0x7ffffe9, 27}, { 0x7ffffea, 27}, //240-247
    { 0x7ffffeb, 27}, { 0xffffffe, 28}, { 0x7ffffec, 27}, { 0x7ffffed, 27}, { 0x7ffffee, 27}, { 0x7ffffef, 27}, { 0x7fffff0, 27}, { 0x3ffffee, 26}, //248-255
    {0x3fffffff, 30}                                                                                                                                //256
};

node *created_huffman_tree();

extern const node *huffman_root;

std::string get_huffman_string(const uint8_t *pstart, const uint8_t *pend);

inline std::string get_header_string(const uint8_t *&pstart) {
    size_t len = *pstart & 0x7f;
    if ((*pstart & 0x80) == 0x80) {
        ++pstart;
        std::string value = get_huffman_string(pstart, pstart + len);
        pstart += len;
        return value;
    } else {
        ++pstart;
        std::string value((char *)pstart, len);
        pstart += len;
        return value;
    }
}

inline http_header::FIELD get_header_field(const uint8_t *&pstart) {
    auto header_string = get_header_string(pstart);
    auto header_itr = http_header::field_map.find(header_string);
    if (header_itr == http_header::field_map.end()) {
        return http_header::FIELD::IGNORE_THIS;
    } else {
        return header_itr->second;
    }
}

inline http_header_request::METHOD get_header_method(const std::string &method_name) {
    auto header_itr = http_header_request::method_map.find(method_name);
    if (header_itr == http_header_request::method_map.end()) {
        return http_header_request::METHOD::IGNORE_THIS;
    } else {
        return header_itr->second;
    }
}


} // namespace rohit::http::v2