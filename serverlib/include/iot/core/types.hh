////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <bit>
#include <algorithm>

struct sockaddr_in6;
namespace rohit {

template <typename T>
constexpr T changeEndian(const T &val) {
    static_assert(
        sizeof(T) == sizeof(uint16_t) || sizeof(T) == sizeof(uint32_t) || sizeof(T) == sizeof(uint64_t),
        "Only type of size 16, 32 and 64 supported by change Endian");
    if constexpr (std::endian::native == std::endian::little) {
        if constexpr (sizeof(T) == sizeof(uint16_t)) return __bswap_16 (val);
        if constexpr (sizeof(T) == sizeof(uint32_t)) return __bswap_32 (val);
        if constexpr (sizeof(T) == sizeof(uint64_t)) return __bswap_64 (val);
    } else if constexpr (std::endian::native == std::endian::big) {
        // static_assert(std::endian::native == std::endian::big, "Code must reach here only for big endian");
        return val;
    } else {
        return val;
    }
}

template <typename T> struct is_int8_t { static constexpr bool const value = false; };
template <> struct is_int8_t<int8_t> { static constexpr bool const value = true; };
template <typename T> struct is_int16_t { static constexpr bool const value = false; };
template <> struct is_int16_t<int16_t> { static constexpr bool const value = true; };

static constexpr const size_t ipv6_addr_size = 16;
static constexpr const size_t ipv6_addr16_size = ipv6_addr_size/sizeof(uint16_t);
static constexpr const size_t ipv6_addr32_size = ipv6_addr_size/sizeof(uint32_t);
static constexpr const size_t ipv6_addr64_size = ipv6_addr_size/sizeof(uint64_t);

union ipv6_addr_t {
    uint8_t     addr_8[ipv6_addr_size];
    uint16_t    addr_16[ipv6_addr16_size];
    uint32_t    addr_32[ipv6_addr32_size];
    uint64_t    addr_64[ipv6_addr64_size];
}  __attribute__((packed));;

class ipv6_port_t {
private:
    uint16_t value;
public:
    constexpr ipv6_port_t(const uint16_t value) : value(changeEndian(value)) {}
    constexpr ipv6_port_t(const ipv6_port_t &rhs) : value(rhs.value) {}
    constexpr operator uint16_t() const { return changeEndian(value); }
    constexpr uint16_t get_network_port() const { return value; }

}  __attribute__((packed));;

class ipv6_socket_addr_t {
public:
    ipv6_addr_t addr;
    ipv6_port_t port;

    constexpr ipv6_socket_addr_t(const ipv6_addr_t &addr, const ipv6_port_t port) : addr(addr), port(port) { }
    constexpr ipv6_socket_addr_t(const void *addr, const ipv6_port_t port) : addr(*(ipv6_addr_t *)addr), port(port) { }
    constexpr ipv6_socket_addr_t(const char *addrstr, const ipv6_port_t port);
    
    constexpr operator sockaddr_in6() const;
}  __attribute__((packed));

class guid_t {
public:
    static constexpr const size_t size = 16;
    static constexpr const size_t size_16 = size/sizeof(uint16_t);
    static constexpr const size_t size_32 = size/sizeof(uint32_t);
    static constexpr const size_t size_64 = size/sizeof(uint64_t);

    static const constexpr std::size_t guid_string_size = 36;
    static const constexpr std::size_t guid_string_withnull_size = 37;

private:
    union {
        uint8_t     guid_8[size];
        uint16_t    guid_16[size_16];
        uint32_t    guid_32[size_32];
        uint64_t    guid_64[ipv6_addr64_size];
    };

public:
    constexpr guid_t() : guid_8() {}
    constexpr guid_t(const uint8_t *guid_binary) : guid_8() { std::copy(guid_binary, guid_binary + size, guid_8); }

    constexpr uint8_t operator[](size_t index) const { return guid_8[index]; }
} __attribute__((packed)); // class guid_t

typedef uint16_t log_id_type;
typedef uint16_t state_type;

enum class state_t : state_type;
enum class err_t : log_id_type;

typedef bool bool_t;
typedef char char_t;
typedef char * string_t;
typedef float float_t;
typedef double double_t;

#define LIST_DEFINITION_END

#define TYPE_LIST \
    TYPE_LIST_ENTRY(char_t) \
    TYPE_LIST_ENTRY(bool_t) \
    TYPE_LIST_ENTRY(int8_t) \
    TYPE_LIST_ENTRY(int16_t) \
    TYPE_LIST_ENTRY(int32_t) \
    TYPE_LIST_ENTRY(int64_t) \
    TYPE_LIST_ENTRY(uint8_t) \
    TYPE_LIST_ENTRY(uint16_t) \
    TYPE_LIST_ENTRY(uint32_t) \
    TYPE_LIST_ENTRY(uint64_t) \
    TYPE_LIST_ENTRY(float_t) \
    TYPE_LIST_ENTRY(double_t) \
    TYPE_LIST_ENTRY(string_t) \
    TYPE_LIST_ENTRY(err_t) \
    TYPE_LIST_ENTRY(state_t) \
    TYPE_LIST_ENTRY(guid_t) \
    TYPE_LIST_ENTRY(ipv6_addr_t) \
    TYPE_LIST_ENTRY(ipv6_port_t) \
    TYPE_LIST_ENTRY(ipv6_socket_addr_t) \
    LIST_DEFINITION_END


enum class type_identifier {
#define TYPE_LIST_ENTRY(x) x,
    TYPE_LIST
#undef TYPE_LIST_ENTRY

    bad_type,
    the_end,
};

constexpr const char * type_str[] = {
#define TYPE_LIST_ENTRY(x) #x,
    TYPE_LIST
#undef TYPE_LIST_ENTRY

    "bad_type",
    "the_end"
};

constexpr const char * to_string(const type_identifier id) {
    return type_str[static_cast<size_t>(id)];
}

template <typename T>
struct what_type
{
    static constexpr const type_identifier value = type_identifier::bad_type;
    static constexpr const char str[] = "bad_type";
};

template <>
struct what_type<const char *>
{
    static constexpr const type_identifier value = type_identifier::string_t;
    static constexpr const char str[] = "string_t";
};

#define TYPE_LIST_ENTRY(x) \
template <> \
struct what_type<x> \
{ \
    static constexpr const type_identifier value = type_identifier::x; \
    static constexpr const char str[] = #x; \
};
    TYPE_LIST
#undef TYPE_LIST_ENTRY

template <>
struct what_type<long long>
{
    static constexpr const type_identifier value = type_identifier::int64_t;
    static constexpr const char str[] = "int64_t";
};

template <>
struct what_type<unsigned long long>
{
    static constexpr const type_identifier value = type_identifier::uint64_t;
    static constexpr const char str[] = "uint64_t";
};

template <typename T, type_identifier type> struct is_type { static constexpr bool const value = false; };
#define TYPE_LIST_ENTRY(x) \
template <> struct is_type<x, type_identifier::x> { static constexpr bool const value = true; };
    TYPE_LIST
#undef TYPE_LIST_ENTRY

template <type_identifier type> struct type_length { static constexpr size_t const value = 0; };
#define TYPE_LIST_ENTRY(x) \
template <> struct type_length<type_identifier::x> { static constexpr size_t const value = sizeof(x); };
    TYPE_LIST
#undef TYPE_LIST_ENTRY

constexpr unsigned long long int operator "" _kb (const unsigned long long int value) { 
    return 1024 * value;
}

constexpr unsigned long long int operator "" _mb (const unsigned long long int value) {
    return 1024_kb * value;
}

constexpr unsigned long long int operator "" _gb (const unsigned long long int value) {
    return 1024_mb * value;
}

constexpr unsigned long long int operator "" _tb (const unsigned long long int value) {
    return 1024_gb * value;
}

constexpr unsigned long long int operator "" _pb (const unsigned long long int value) {
    return 1024_tb * value;
}

} // namespace rohit 