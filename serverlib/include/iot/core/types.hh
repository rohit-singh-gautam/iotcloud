#pragma once

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

namespace rohit {

template <typename T> struct is_int8_t { static constexpr bool const value = false; };
template <> struct is_int8_t<int8_t> { static constexpr bool const value = true; };
template <typename T> struct is_int16_t { static constexpr bool const value = false; };
template <> struct is_int16_t<int16_t> { static constexpr bool const value = true; };

class ipv6_addr;

enum class type_identifier {
    char_t,
    int8_t,
    int16_t,
    int32_t,
    int64_t,
    uint8_t,
    uint16_t,
    uint32_t,
    uint64_t,
    float_t,
    double_t,
    size_t,
    ssize_t,
    ipv6_addr_t,
    bad_type,
    the_end
};

constexpr const char * type_str[] = {
    "char_t",
    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    "float_t",
    "double_t",
    "size_t",
    "ssize_t",
    "ipv6_addr_t",
    "bad_type",
    "the_end" };

template <typename T>
struct what_type
{
    static constexpr const type_identifier value = type_identifier::bad_type;
    static constexpr const char str[] = "bad_type";
};
template <>
struct what_type<char>
{
    static constexpr const type_identifier value = type_identifier::char_t;
    static constexpr const char str[] = "char_t";
};
template <>
struct what_type<int8_t>
{
    static constexpr const type_identifier value = type_identifier::int8_t;
    static constexpr const char str[] = "int8_t";
};
template <>
struct what_type<int16_t>
{
    static constexpr const type_identifier value = type_identifier::int16_t;
    static constexpr const char str[] = "int16_t";
};
template <>
struct what_type<int32_t>
{
    static constexpr const type_identifier value = type_identifier::int32_t;
    static constexpr const char str[] = "int32_t";
};
template <>
struct what_type<int64_t>
{
    static constexpr const type_identifier value = type_identifier::int64_t;
    static constexpr const char str[] = "int64_t";
};
template <>
struct what_type<long long>
{
    static constexpr const type_identifier value = type_identifier::int64_t;
    static constexpr const char str[] = "int64_t";
};
template <>
struct what_type<uint8_t>
{
    static constexpr const type_identifier value = type_identifier::uint8_t;
    static constexpr const char str[] = "uint8_t";
};
template <>
struct what_type<uint16_t>
{
    static constexpr const type_identifier value = type_identifier::uint16_t;
    static constexpr const char str[] = "uint16_t";
};
template <>
struct what_type<uint32_t>
{
    static constexpr const type_identifier value = type_identifier::uint32_t;
    static constexpr const char str[] = "uint32_t";
};
template <>
struct what_type<uint64_t>
{
    static constexpr const type_identifier value = type_identifier::uint64_t;
    static constexpr const char str[] = "uint64_t";
};
template <>
struct what_type<unsigned long long>
{
    static constexpr const type_identifier value = type_identifier::uint64_t;
    static constexpr const char str[] = "uint64_t";
};
template <>
struct what_type<float>
{
    static constexpr const type_identifier value = type_identifier::float_t;
    static constexpr const char str[] = "float_t";
};
template <>
struct what_type<double>
{
    static constexpr const type_identifier value = type_identifier::double_t;
    static constexpr const char str[] = "double_t";
};
template <>
struct what_type<ipv6_addr>
{
    static constexpr const type_identifier value = type_identifier::ipv6_addr_t;
    static constexpr const char str[] = "ipv6_addr_t";
};

template <typename T, type_identifier type> struct is_type { static constexpr bool const value = false; };
template <> struct is_type<char, type_identifier::char_t> { static constexpr bool const value = true; };
template <> struct is_type<int8_t, type_identifier::int8_t> { static constexpr bool const value = true; };
template <> struct is_type<int16_t, type_identifier::int16_t> { static constexpr bool const value = true; };
template <> struct is_type<int32_t, type_identifier::int32_t> { static constexpr bool const value = true; };
template <> struct is_type<int64_t, type_identifier::int64_t> { static constexpr bool const value = true; };
template <> struct is_type<uint8_t, type_identifier::uint8_t> { static constexpr bool const value = true; };
template <> struct is_type<uint16_t, type_identifier::uint16_t> { static constexpr bool const value = true; };
template <> struct is_type<uint32_t, type_identifier::uint32_t> { static constexpr bool const value = true; };
template <> struct is_type<uint64_t, type_identifier::uint64_t> { static constexpr bool const value = true; };
template <> struct is_type<float, type_identifier::float_t> { static constexpr bool const value = true; };
template <> struct is_type<double, type_identifier::double_t> { static constexpr bool const value = true; };
template <> struct is_type<uint64_t, type_identifier::size_t> { static constexpr bool const value = true; };
template <> struct is_type<int64_t, type_identifier::ssize_t> { static constexpr bool const value = true; };
template <> struct is_type<ipv6_addr, type_identifier::ipv6_addr_t> { static constexpr bool const value = true; };

template <type_identifier type> struct type_length { static constexpr size_t const value = 0; };
template <> struct type_length<type_identifier::char_t> { static constexpr size_t const value = sizeof(char); };
template <> struct type_length<type_identifier::int8_t> { static constexpr size_t const value = sizeof(int8_t); };
template <> struct type_length<type_identifier::int16_t> { static constexpr size_t const value = sizeof(int16_t); };
template <> struct type_length<type_identifier::int32_t> { static constexpr size_t const value = sizeof(int32_t); };
template <> struct type_length<type_identifier::int64_t> { static constexpr size_t const value = sizeof(int64_t); };
template <> struct type_length<type_identifier::uint8_t> { static constexpr size_t const value = sizeof(uint8_t); };
template <> struct type_length<type_identifier::uint16_t> { static constexpr size_t const value = sizeof(uint16_t); };
template <> struct type_length<type_identifier::uint32_t> { static constexpr size_t const value = sizeof(uint32_t); };
template <> struct type_length<type_identifier::uint64_t> { static constexpr size_t const value = sizeof(uint64_t); };
template <> struct type_length<type_identifier::float_t> { static constexpr size_t const value = sizeof(float); };
template <> struct type_length<type_identifier::double_t> { static constexpr size_t const value = sizeof(double); };
template <> struct type_length<type_identifier::size_t> { static constexpr size_t const value = sizeof(size_t); };
template <> struct type_length<type_identifier::ssize_t> { static constexpr size_t const value = sizeof(ssize_t); };


typedef uint16_t log_id_type;
typedef uint16_t state_type;

} // namespace rohit 