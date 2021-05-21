#pragma once
#include <stdint.h>
#include <stddef.h>
#include <type_traits>
#include <cmath>

namespace iotcloud {

namespace math {

inline constexpr void reverse(char *start, char *end) {
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

template <typename T, T radix = 10, number_case number_case = number_case::lower, bool null_terminated = true>
constexpr size_t integerToString(char *dest, T val) {
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

} // namespace math

} // namespace iotcloud