////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once
#include <http11.hh>
#include <string>
#include <vector>
#include <iot/core/config.hh>
#include <iot/core/error.hh>

namespace std {
template<>
struct hash<std::pair<rohit::http_header::FIELD, std::string>>
{
    size_t
    operator()(const std::pair<rohit::http_header::FIELD, std::string> &val) const noexcept
    {
        return std::_Hash_impl::__hash_combine(val.first, std::hash<std::string>{}(val.second));
    }
};
} // namespace std

namespace rohit::http::v2 {

class map_table_t {
private:
    std::vector<std::pair<http_header::FIELD, std::string>> entries;
    std::unordered_map<std::pair<http_header::FIELD, std::string>, size_t> entry_value_map;

    // Field string -> index, count
    // Count will be used for dynamic to cleanup
    std::unordered_map<http_header::FIELD, std::pair<size_t, size_t>> entry_map;

public:
    inline map_table_t() : entries(), entry_value_map(), entry_map() {}
    inline map_table_t(const std::initializer_list<std::pair<http_header::FIELD, std::string>> &list)
                : entries(), entry_value_map(), entry_map() {
        for(auto &entry: list) {
            push_back(entry);
        }
    }

    inline std::pair<http_header::FIELD, std::string> operator[](const size_t index) const {
        if (index >= entries.size()) {
            return {http_header::FIELD::IGNORE_THIS, ""};
        } else {
            return entries[index];
        }
    }

    inline size_t operator[](const std::pair<http_header::FIELD, std::string> &header_line) const {
        auto entry_itr = entry_value_map.find(header_line);
        if (entry_itr == entry_value_map.end()) {
            return -1;
        }
        return entry_itr->second;
    }

    inline size_t operator[](const http_header::FIELD &field) const {
        auto entry_itr = entry_map.find(field);
        if (entry_itr == entry_map.end()) {
            return -1;
        }
        return entry_itr->second.first;
    }

    inline void push_back(const std::pair<http_header::FIELD, std::string> &entry) {
        entries.push_back(entry);
        const size_t index = entries.size() - 1;
        if (entry.second != "") {
            entry_value_map.insert(std::make_pair(entry, index));
        }
        auto entry_itr = entry_map.find(entry.first);
        if (entry_itr == entry_map.end()) {
            entry_map.insert(std::make_pair(entry.first, std::make_pair(index, 1)));
        } else {
            entry_itr->second.second++;
        }
    }

    inline void pop_back() {
        const auto entry = entries.back();
        entries.pop_back();
        
        if (entry.second != "") {
            entry_value_map.erase(entry);
        }

        if constexpr (config::debug) {
            auto entry_itr = entry_map.find(entry.first);
            if (entry_itr != entry_map.end()) {
                if (entry_itr->second.second == 1) {
                    entry_map.erase(entry.first);
                } else {
                    entry_itr->second.second--;
                }
            } else {
                throw exception_t(err_t::HTTP2_HPACK_TABLE_ERROR);
            }
        } else {
            auto entry_itr = entry_map.find(entry.first);
            if (entry_itr->second.second == 1) {
                entry_map.erase(entry.first);
            } else {
                entry_itr->second.second--;
            }
        }
    }

    inline bool contains(const std::pair<http_header::FIELD, std::string> &entry) const {
        return entry_value_map.contains(entry);
    }

    inline bool contains(const http_header::FIELD &entry) const {
        return entry_map.contains(entry);
    }

    inline void clear() {
        entries.clear();
        entry_value_map.clear();
        entry_map.clear();
    }

    inline size_t size() const {
        return entries.size();
    }

};

class dynamic_table_t : public map_table_t {
private:
    size_t max_size;

public:
    dynamic_table_t(size_t max_size = 12) : map_table_t(), max_size(max_size) {}

    inline void update_size(size_t new_size) {
        if (max_size == new_size) return;
        max_size = new_size;
        while (new_size < size()) {
            pop_back();
        }
    }

    inline void insert(const std::pair<http_header::FIELD, std::string> &entry) {
        if (size() >= max_size) {
            // rfc7541 - 4.4.  Entry Eviction When Adding New Entries
            clear();
        }
        push_back(entry);
    }
};

#define HTTP2_STATIC_TABLE_LIST \
    HTTP2_STATIC_TABLE_ENTRY( 0, http_header::FIELD::IGNORE_THIS, "") /* 0 is not used */ \
    HTTP2_STATIC_TABLE_ENTRY( 1, http_header::FIELD::Authority, "") \
    HTTP2_STATIC_TABLE_ENTRY( 2, http_header::FIELD::Method, "GET") \
    HTTP2_STATIC_TABLE_ENTRY( 3, http_header::FIELD::Method, "POST") \
    HTTP2_STATIC_TABLE_ENTRY( 4, http_header::FIELD::Path, "/") \
    HTTP2_STATIC_TABLE_ENTRY( 5, http_header::FIELD::Path, "/index.html") \
    HTTP2_STATIC_TABLE_ENTRY( 6, http_header::FIELD::Scheme,  "http") \
    HTTP2_STATIC_TABLE_ENTRY( 7, http_header::FIELD::Scheme, "https") \
    HTTP2_STATIC_TABLE_ENTRY( 8, http_header::FIELD::Status, "200") \
    HTTP2_STATIC_TABLE_ENTRY( 9, http_header::FIELD::Status, "204") \
    HTTP2_STATIC_TABLE_ENTRY(10, http_header::FIELD::Status, "206") /* 10 */ \
    HTTP2_STATIC_TABLE_ENTRY(11, http_header::FIELD::Status, "304") \
    HTTP2_STATIC_TABLE_ENTRY(12, http_header::FIELD::Status, "400") \
    HTTP2_STATIC_TABLE_ENTRY(13, http_header::FIELD::Status, "404") \
    HTTP2_STATIC_TABLE_ENTRY(14, http_header::FIELD::Status, "500") \
    HTTP2_STATIC_TABLE_ENTRY(15, http_header::FIELD::Accept_Charset, "") \
    HTTP2_STATIC_TABLE_ENTRY(16, http_header::FIELD::Accept_Encoding, "gzip, deflate") \
    HTTP2_STATIC_TABLE_ENTRY(17, http_header::FIELD::Accept_Language, "") \
    HTTP2_STATIC_TABLE_ENTRY(18, http_header::FIELD::Accept_Ranges, "") \
    HTTP2_STATIC_TABLE_ENTRY(19, http_header::FIELD::Accept, "") \
    HTTP2_STATIC_TABLE_ENTRY(20, http_header::FIELD::Access_Control_Allow_Origin, "") /* 20 */ \
    HTTP2_STATIC_TABLE_ENTRY(21, http_header::FIELD::Age, "") \
    HTTP2_STATIC_TABLE_ENTRY(22, http_header::FIELD::Allow, "") \
    HTTP2_STATIC_TABLE_ENTRY(23, http_header::FIELD::Authorization, "") \
    HTTP2_STATIC_TABLE_ENTRY(24, http_header::FIELD::Cache_Control, "") \
    HTTP2_STATIC_TABLE_ENTRY(25, http_header::FIELD::Content_Disposition, "") \
    HTTP2_STATIC_TABLE_ENTRY(26, http_header::FIELD::Content_Encoding, "") \
    HTTP2_STATIC_TABLE_ENTRY(27, http_header::FIELD::Content_Language, "") \
    HTTP2_STATIC_TABLE_ENTRY(28, http_header::FIELD::Content_Length, "") \
    HTTP2_STATIC_TABLE_ENTRY(29, http_header::FIELD::Content_Location, "") \
    HTTP2_STATIC_TABLE_ENTRY(30, http_header::FIELD::Content_Range, "") /* 30 */ \
    HTTP2_STATIC_TABLE_ENTRY(31, http_header::FIELD::Content_Type, "") \
    HTTP2_STATIC_TABLE_ENTRY(32, http_header::FIELD::Cookie, "") \
    HTTP2_STATIC_TABLE_ENTRY(33, http_header::FIELD::Date, "") \
    HTTP2_STATIC_TABLE_ENTRY(34, http_header::FIELD::ETag, "") \
    HTTP2_STATIC_TABLE_ENTRY(35, http_header::FIELD::Expect, "") \
    HTTP2_STATIC_TABLE_ENTRY(36, http_header::FIELD::Expires, "") \
    HTTP2_STATIC_TABLE_ENTRY(37, http_header::FIELD::From, "") \
    HTTP2_STATIC_TABLE_ENTRY(38, http_header::FIELD::Host, "") \
    HTTP2_STATIC_TABLE_ENTRY(39, http_header::FIELD::If_Match, "") \
    HTTP2_STATIC_TABLE_ENTRY(40, http_header::FIELD::If_Modified_Since, "") /* 40 */ \
    HTTP2_STATIC_TABLE_ENTRY(41, http_header::FIELD::If_None_Match, "") \
    HTTP2_STATIC_TABLE_ENTRY(42, http_header::FIELD::If_Range, "") \
    HTTP2_STATIC_TABLE_ENTRY(43, http_header::FIELD::If_Unmodified_Since, "") \
    HTTP2_STATIC_TABLE_ENTRY(44, http_header::FIELD::Last_Modified, "") \
    HTTP2_STATIC_TABLE_ENTRY(45, http_header::FIELD::Link, "") \
    HTTP2_STATIC_TABLE_ENTRY(46, http_header::FIELD::Location, "") \
    HTTP2_STATIC_TABLE_ENTRY(47, http_header::FIELD::Max_Forwards, "") \
    HTTP2_STATIC_TABLE_ENTRY(48, http_header::FIELD::Proxy_Authenticate, "") \
    HTTP2_STATIC_TABLE_ENTRY(49, http_header::FIELD::Proxy_Authorization, "") \
    HTTP2_STATIC_TABLE_ENTRY(50, http_header::FIELD::Range, "") /* 50 */ \
    HTTP2_STATIC_TABLE_ENTRY(51, http_header::FIELD::Referer, "") \
    HTTP2_STATIC_TABLE_ENTRY(52, http_header::FIELD::Refresh, "") \
    HTTP2_STATIC_TABLE_ENTRY(53, http_header::FIELD::Retry_After, "") \
    HTTP2_STATIC_TABLE_ENTRY(54, http_header::FIELD::Server, "") \
    HTTP2_STATIC_TABLE_ENTRY(55, http_header::FIELD::Set_Cookie, "") \
    HTTP2_STATIC_TABLE_ENTRY(56, http_header::FIELD::Strict_Transport_Security, "") \
    HTTP2_STATIC_TABLE_ENTRY(57, http_header::FIELD::Transfer_Encoding, "") \
    HTTP2_STATIC_TABLE_ENTRY(58, http_header::FIELD::User_Agent, "") \
    HTTP2_STATIC_TABLE_ENTRY(59, http_header::FIELD::Vary, "") \
    HTTP2_STATIC_TABLE_ENTRY(60, http_header::FIELD::Via, "") /* 60 */ \
    HTTP2_STATIC_TABLE_ENTRY(61, http_header::FIELD::WWW_Authenticate, "") \
    LIST_DEFINITION_END

class static_table_t : public map_table_t {
public:
    using map_table_t::map_table_t;
};

extern const static_table_t static_table;

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

template <uint32_t N>
constexpr uint32_t decode_integer(const uint8_t *&pstart, const uint8_t *pend) {
    constexpr uint32_t mask = (1 << N) - 1;
    uint32_t value = *pstart++ & mask;
    if (value < mask) {
        return value;
    }
    value = mask;
    while(pstart < pend) {
        if ((*pstart & 0x80) != 0) {
            value = *pstart++ & 0x7f;
        } else {
            value = *pstart++;
            break;
        }
    }
    return value;
}

// Assuming buffer has sufficient data, hence no check
template <uint32_t N>
constexpr uint8_t * encode_integer(uint8_t *pstart, const uint8_t head, uint32_t value) {
    constexpr uint32_t mask = (1 << N) - 1;
    if (value < mask) {
        *pstart++ = head + (uint8_t)value;
    } else {
        *pstart++ = head + mask;
        value -= mask;
        while(value >= 128) {
            *pstart++ = 0x80 & (value % 128);
            value >>= 7;
        }
        *pstart++ = value;
    }
    return pstart;
}


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

inline size_t huffman_string_size(const std::string &value) {
    size_t size = 8;
    for(auto &ch: value) {
        size += static_huffman[ch].code_len;
    }
    return size / 8;
}

uint8_t *add_huffman_string(uint8_t *pstart, const std::string &value);

inline uint8_t *add_header_string(uint8_t *pstart, const std::string &value) {
    size_t size = huffman_string_size(value);
    if (size < value.size()) {
        // Encoded string is smaller hence we are encoded
        pstart = encode_integer<7>(pstart, (uint8_t)0x80, (uint32_t)size);
        pstart = add_huffman_string(pstart, value);
    } else {
        pstart = encode_integer<7>(pstart, (uint8_t)0x80, (uint32_t)value.size());
        pstart = std::copy(value.begin(), value.end(), pstart);
    }

    return pstart;
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