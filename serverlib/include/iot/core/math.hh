////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <type_traits>
#include <cmath>

namespace rohit {

constexpr void reverse(char *start, char *end) {
    while(start < end) {
        std::swap(*start, *end);
        ++start; --end;
    }
}

enum class number_case {
    lower,
    upper
};

constexpr char lower_case_numbers[] = {
    '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','g','h',
    'i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
constexpr char upper_case_numbers[] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H',
    'I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

constexpr uint8_t char_to_int[] {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 00  - 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 16  - 31
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 32  - 47
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 48  - 63
    0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 64  - 79
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 80  - 95
    0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 96  - 111
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 112 - 127
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 128 - 143
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 144 - 159
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 160 - 175
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 176 - 191
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 192 - 207
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 208 - 223
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 224 - 239
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 240 - 255
};

template <typename T, T radix = 10, number_case number_case = number_case::lower, bool null_terminated = true>
constexpr size_t to_string(T val, char * const dest) {
    static_assert(std::is_integral_v<T>, "Only integral type allowed");
    static_assert(!std::is_signed<T>::value || (std::is_signed<T>::value && radix == 10), "Signed type only allowed for radix 10" );
    static_assert(radix >= 2, "Radix must be atleast 2");
    static_assert(radix <= 36, "Radix more than 36 not supported");
    char *dest_ptr = dest;

    T val1;
    if constexpr (std::is_signed<T>::value) val1 = abs(val);
    else val1 = val;

    do {
        T modval = val1 % radix;
        if constexpr (number_case == number_case::lower)
            *dest_ptr++ = lower_case_numbers[modval];
        else
            *dest_ptr++ = upper_case_numbers[modval];
    } while((val1 /= radix));

    if constexpr (std::is_signed<T>::value) {
        if (val < 0) *dest_ptr++ = '-';
    }
    reverse(dest , dest_ptr - 1);

    if constexpr (null_terminated == true) {
        *dest_ptr++ = '\0';
    }
    return (size_t)(dest_ptr - dest);
}

template <typename T, bool null_terminated = true> 
constexpr size_t floatToString(char *dest, T val) {
    static_assert(std::is_floating_point_v<T>, "Only floating point type allowed");
    return sprintf(dest, "%f", val);
}

template <typename T, T radix = 10>
constexpr T to_uint(const char *src, size_t *len = nullptr) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "Only unsigned integral type allowed");
    T val = 0;
    char current = 0;
    const char *psrc = src;
    if ((current = *psrc++)) {
        val = char_to_int[(size_t)current];
        while((current = *psrc++)) {
            val = val * radix + char_to_int[(size_t)current];
        }
    }

    if (len != nullptr) *len = (size_t)(psrc - src);

    return val;
}

template <typename T, T radix = 10>
constexpr T to_int(const char *src, size_t *len = nullptr) {
    static_assert(std::is_integral_v<T> && std::is_signed_v<T>, "Only signed integral type allowed");
    T val = 0;
    char current = 0;
    const char *psrc = src;

    //Checking for sign
    T sign = 1;
    switch(*psrc) {
    case '-':
        sign = -1;
    case '+':
        ++psrc;
        break;
    default:
        break;
    }

    if ((current = *psrc++)) {
        val = char_to_int[(size_t)current];
        while((current = *psrc++)) {
            val = val * radix + char_to_int[(size_t)current];
        }
    }

    if (len != nullptr) *len = (size_t)(psrc - src);

    return val * sign;
}

} // namespace rohit