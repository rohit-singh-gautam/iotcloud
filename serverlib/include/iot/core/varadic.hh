////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include "types.hh"
#include <assert.h>
#include <iostream>

namespace rohit {

template <typename T, typename... ARGS>
constexpr size_t sizeofvaargs(const T& arg, const ARGS&... args) {
    static_assert(!std::is_pointer_v<T>, "Pointer type of variable argument no supported");
    return sizeof(arg) + sizeofvaargs(args...);
}

template <typename T>
constexpr void copyvaradic(uint8_t * const arr, const T& arg) {
    uint8_t * const arrsrc = (uint8_t * const)(&arg);
    std::copy(arrsrc, arrsrc + sizeof(T), arr);
}

template <typename T, typename... ARGS>
constexpr void copyvaradic(uint8_t * const arr, const T& arg, const ARGS&... args) {
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

enum class formatstring_state { 
    COPY,
    MODIFIER,
    MODIFIER_CUSTOM
};

enum class formatstring_type_length {
    NONE,
    h,
    hh,
    l,
    ll
};

constexpr size_t formatstring_count(const char *arr) {
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
                ++count;
                state = formatstring_state::COPY;
                break;
            case 'v':
                state = formatstring_state::MODIFIER_CUSTOM;
            }
            break;

        case formatstring_state::MODIFIER_CUSTOM:
            switch(c) {
            case 'n':
            case 'N':
            case 'i':
            case 'I':
            case 'p':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
            case 'v':
            case 's':
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
                    state = formatstring_state::MODIFIER_CUSTOM;
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

                default:
                    type_list[index++] = type_identifier::bad_type; break;
                } //switch (c)
                
                break; // case formatstring_state::MODIFIER:

            case formatstring_state::MODIFIER_CUSTOM:
                switch(c) {
                case 'n':
                case 'N':
                    type_list[index++] = type_identifier::ipv6_socket_addr_t;
                    length += type_length<type_identifier::ipv6_socket_addr_t>::value;
                    break;
                case 'i':
                case 'I':
                    type_list[index++] = type_identifier::ipv6_addr_t;
                    length += type_length<type_identifier::ipv6_addr_t>::value;
                    break;
                case 'p':
                    type_list[index++] = type_identifier::ipv6_port_t;
                    length += type_length<type_identifier::ipv6_port_t>::value;
                    break;
                case 'e':
                    type_list[index++] = type_identifier::int32_t;
                    length += type_length<type_identifier::int32_t>::value;
                    break;
                case 'E':
                    type_list[index++] = type_identifier::err_t;
                    length += type_length<type_identifier::err_t>::value;
                    break;
                case 'g':
                case 'G':
                    type_list[index++] = type_identifier::guid_t;
                    length += type_length<type_identifier::guid_t>::value;
                    break;
                case 'v':
                    type_list[index++] = type_identifier::uint32_t;
                    length += type_length<type_identifier::uint32_t>::value;
                    break;
                case 's':
                    type_list[index++] = type_identifier::state_t;
                    length += type_length<type_identifier::state_t>::value;
                    break;
                default:
                        type_list[index++] = type_identifier::bad_type; break;
                }
                state = formatstring_state::COPY;
                break; // case formatstring_state::MODIFIER_CUSTOM:
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

template <const size_t COUNT, formatstring_type_list<COUNT> fmt_list, typename T, typename... ARGS>
constexpr size_t check_formatstring_args_internal()
{
    constexpr const size_t index = COUNT - sizeof...(ARGS) - 1;
    if (fmt_list[index] != what_type<T>::value) {
        //std::cout << "Index: " << index << ": what_type: " << what_type<T>::str << ", fmt_list: " << type_str[(int)fmt_list[index]] << std::endl;
        return index; // Bad type
    }

    if constexpr (sizeof...(ARGS) >= 1) {
        return check_formatstring_args_internal<COUNT, fmt_list, ARGS...>();
    }
    return SIZE_MAX;
}

template <const size_t COUNT, formatstring_type_list<COUNT> fmt_list, typename... ARGS>
constexpr size_t check_formatstring_args() {
    if constexpr (COUNT != sizeof...(ARGS)) {
        //std::cout << "COUNT: " << COUNT << ", sizeof...(ARGS): " << sizeof...(ARGS) << std::endl;
        return 0;
    }
    if constexpr (sizeof...(ARGS) >= 1) return check_formatstring_args_internal<COUNT, fmt_list, ARGS...>();
    if constexpr (COUNT >= 1) return 0;
    return SIZE_MAX;
}

} // namespace rohit