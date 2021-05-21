#pragma once
#include "iotcloudtypes.hh"
#include "socket.hh"
#include <assert.h>
#include <iostream>

namespace iotcloud {

template <typename T>
inline constexpr size_t sizeofvaargs(const T& arg) {
    return sizeof(arg);
}

template <typename T, typename... ARGS>
inline constexpr size_t sizeofvaargs(const T& arg, const ARGS&... args) {
    return sizeofvaargs(arg) + sizeofvaargs(args...);
}

template <typename T>
inline constexpr void copyvaradic(uint8_t *arr, const T& arg) {
    *(T *)arr = arg;
}

template <>
inline constexpr void copyvaradic<>(uint8_t *arr, const ipv6_addr& arg) {
    for(size_t count = 0; count < sizeof(ipv6_addr); count++) {
        *(arr + count) = *((uint8_t *)&arg + count);
    }
}

template <typename T, typename... ARGS>
inline constexpr void copyvaradic(uint8_t *arr, const T& arg, const ARGS&... args) {
    copyvaradic(arr, arg);
    copyvaradic(arr + sizeof(arg), args...);
}

template <typename T, typename ...ARGS>
class sizeofvaradic {
public:
    static const constexpr size_t size = sizeof(T) + sizeofvaradic<ARGS...>::size;
};

template <typename T>
class sizeofvaradic<T> {
public:
    static const constexpr size_t size = sizeof(T);
};

// Supported format specifier
// %d or %i - signed integer (int32_t)
// %u - unsigned integer (uint32_t)
// %o - unsigned integer (uint32_t)(Octal)
// %x - unsigned integer (uint32_t)(hex small case)
// %X - unsigned integer (uint32_t)(hex capital case)
// %f - floating point lower case (float)
// %F - floating point upper case (float)
// %c - character (char)
// %v - IPv6 Address format
// %% - %
//
// Supported format length
// h - short - 16 bits
// hh - ultra short 8 bits
// l - long
// ll - long long
// z - size_t

enum class formatstring_state { 
    COPY,
    MODIFIER
};

enum class formatstring_type_length {
    NONE,
    h,
    hh,
    l,
    ll,
    z
};

inline constexpr size_t formatstring_count(const char *arr) {
    formatstring_state state = formatstring_state::COPY;
    int count = 0;

    while (*arr) {
        const char c = *arr++;
        switch (state) {
        case formatstring_state::COPY:
            if (c == '%') {
                state = formatstring_state::MODIFIER;
            }
            break;
        
        case formatstring_state::MODIFIER:
            switch (c)
            {
            case '%':
                state = formatstring_state::COPY;
                break;
            
            case 'c':
            case 'd':
            case 'i':
            case 'u':
            case 'o':
            case 'x':
            case 'X':
            case 'f':
            case 'F':
            case 'v':
                ++count;
                state = formatstring_state::COPY;
                break;
            }
            
            break;
        }
    }

    return count;
}

template <const size_t COUNT> struct formatstring_type_list {
    type_identifier type_list[COUNT];
    size_t length;

    constexpr formatstring_type_list(const char *arr) : type_list(), length(0) {
        size_t index = 0;
        formatstring_state state = formatstring_state::COPY;
        formatstring_type_length lenght_specifier = formatstring_type_length::NONE;

        while (*arr) {
            const char c = *arr++;
            switch (state) {
            case formatstring_state::COPY:
                if (c == '%') {
                    state = formatstring_state::MODIFIER;
                    lenght_specifier = formatstring_type_length::NONE;
                }
                break;
            
            case formatstring_state::MODIFIER:
                switch (c)
                {
                case '%':
                    state = formatstring_state::COPY;
                    break;
                
                case 'c':
                    type_list[index++] = type_identifier::char_t;
                    length += type_length<type_identifier::char_t>::value;
                    state = formatstring_state::COPY;
                    break;

                case 'd':
                case 'i':
                    switch (lenght_specifier) {
                    case formatstring_type_length::NONE:
                        type_list[index++] = type_identifier::int32_t;
                        length += type_length<type_identifier::int32_t>::value;
                        break;
                    case formatstring_type_length::h:
                        type_list[index++] = type_identifier::int16_t;
                        length += type_length<type_identifier::int16_t>::value;
                        break;
                    case formatstring_type_length::hh:
                        type_list[index++] = type_identifier::int8_t;
                        length += type_length<type_identifier::int8_t>::value;
                        break;
                    case formatstring_type_length::l:
                    case formatstring_type_length::ll:
                        type_list[index++] = type_identifier::int64_t;
                        length += type_length<type_identifier::int64_t>::value;
                        break;
                    case formatstring_type_length::z:
                        type_list[index++] = type_identifier::uint64_t;
                        length += type_length<type_identifier::uint64_t>::value;
                        break;
                    }
                    state = formatstring_state::COPY;
                    break;

                case 'u':
                case 'o':
                case 'x':
                case 'X':
                    switch (lenght_specifier) {
                    case formatstring_type_length::NONE:
                        type_list[index++] = type_identifier::uint32_t;
                        length += type_length<type_identifier::uint32_t>::value;
                        break;
                    case formatstring_type_length::h:
                        type_list[index++] = type_identifier::uint16_t;
                        length += type_length<type_identifier::uint16_t>::value;
                        break;
                    case formatstring_type_length::hh:
                        type_list[index++] = type_identifier::uint8_t;
                        length += type_length<type_identifier::uint8_t>::value;
                        break;
                    case formatstring_type_length::l:
                    case formatstring_type_length::ll:
                        type_list[index++] = type_identifier::uint64_t;
                        length += type_length<type_identifier::uint64_t>::value;
                        break;
                    case formatstring_type_length::z:
                        type_list[index++] = type_identifier::uint64_t;
                        length += type_length<type_identifier::uint64_t>::value;
                        break;
                    }
                    state = formatstring_state::COPY;
                    break;
                
                case 'f':
                case 'F':
                    switch (lenght_specifier)
                    {
                    case formatstring_type_length::NONE:
                        type_list[index++] = type_identifier::float_t;
                        length += type_length<type_identifier::float_t>::value;
                        break;
                    case formatstring_type_length::l:
                        type_list[index++] = type_identifier::double_t;
                        length += type_length<type_identifier::double_t>::value;
                        break;
                    default:
                        type_list[index++] = type_identifier::bad_type;
                        break;
                    }
                    state = formatstring_state::COPY;
                    break;

                case 'v':
                    if(lenght_specifier != formatstring_type_length::NONE) {
                        type_list[index++] = type_identifier::bad_type;
                        assert(true);
                        continue;
                    }
                    type_list[index++] = type_identifier::ipv6_addr_t;
                    length += type_length<type_identifier::ipv6_addr_t>::value;
                    state = formatstring_state::COPY;
                    break;

                // Specifiers
                case 'h':
                    if(lenght_specifier != formatstring_type_length::NONE &&
                    lenght_specifier != formatstring_type_length::h) {
                        type_list[index++] = type_identifier::bad_type;
                        assert(true);
                        continue;
                    }
                    if (lenght_specifier == formatstring_type_length::h)
                        lenght_specifier = formatstring_type_length::hh;
                    else lenght_specifier = formatstring_type_length::h;
                    break;
                
                case 'l':
                    if(lenght_specifier != formatstring_type_length::NONE &&
                    lenght_specifier != formatstring_type_length::l) {
                        type_list[index++] = type_identifier::bad_type;
                        assert(true);
                        continue;
                    }
                    if (lenght_specifier == formatstring_type_length::l)
                        lenght_specifier = formatstring_type_length::ll;
                    else lenght_specifier = formatstring_type_length::l;
                    break;

                case 'z':
                    if(lenght_specifier != formatstring_type_length::NONE) {
                        type_list[index++] = type_identifier::bad_type;
                        assert(true);
                        continue;
                    }
                    lenght_specifier = formatstring_type_length::z;
                    break;

                default:
                    type_list[index++] = type_identifier::bad_type; break;
                }
                
                break;
            }
        }

        if( state != formatstring_state::COPY) type_list[index++] = type_identifier::bad_type;
    }

    constexpr bool type_list_check() const {
        for(auto entry: type_list) {
            if (entry == type_identifier::bad_type) {
                return false;
            }
        }

        return true;
    }

    constexpr type_identifier operator[] (size_t index) const { return type_list[index]; }
};

// It will return SIZE_MAX on success and index of failed argument on failure
template <const size_t COUNT, formatstring_type_list<COUNT> fmt_list, typename T>
inline constexpr size_t check_formatstring_args_internal(const size_t index, const T &) {
    if (index >= COUNT) {
        //std::cout << "Too many argument";
        return index; // Too many argument
    }
    
    if (fmt_list[index] != what_type<T>::value) {
        //std::cout << "Index: " << index << ": what_type: " << what_type<T>::str << ", fmt_list: " << type_str[(int)fmt_list[index]] << std::endl;
        return index; // Bad type
    }

    if (index + 1 != COUNT) {
        //std::cout << "Too few argument";
        return index; // Too few argument
    }
    return SIZE_MAX;    
}

template <const size_t COUNT, formatstring_type_list<COUNT> fmt_list, typename T, typename... ARGS>
inline constexpr size_t check_formatstring_args_internal(const size_t index, const T &, const ARGS &...args)
{
    if (index >= COUNT)
        return index; // Too many argument
    if (fmt_list[index] != what_type<T>::value) {
        //std::cout << "Index: " << index << ": what_type: " << what_type<T>::str << ", fmt_list: " << type_str[(int)fmt_list[index]] << std::endl;
        return index; // Bad type
    }
    return check_formatstring_args_internal<COUNT, fmt_list>(index + 1, args...);
}

template <const size_t COUNT, formatstring_type_list<COUNT> fmt_list>
inline constexpr size_t check_formatstring_args() {
    if constexpr (COUNT == 0) return SIZE_MAX;
    else return 0;
}

template <const size_t COUNT, formatstring_type_list<COUNT> fmt_list, typename... ARGS>
inline constexpr size_t check_formatstring_args(const ARGS&... args) {
    return check_formatstring_args_internal<COUNT, fmt_list>((size_t)0, args...);
}

} // namespace iotcloud