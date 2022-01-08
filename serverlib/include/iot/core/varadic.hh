////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include "types.hh"
#include <assert.h>
#include <iostream>
#include <string_view>

namespace rohit {

template <typename T>
concept non_pointer = !std::is_pointer_v<T>;

template <non_pointer ... ARGS>
constexpr size_t sizeofvaargs(const ARGS&... args) {
    return (0 + ... + sizeof(args));
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
    SPECIFIER,
    SPECIFIER_CUSTOM
};

enum class formatstring_modifier {
    NONE,
    h,
    hh,
    l,
    ll,
    L
};

template <size_t size>
constexpr size_t formatstring_count(const char (&fmtstr)[size]) {
    formatstring_state state = formatstring_state::COPY;
    int count = 0;

    for(auto c: fmtstr) {
        switch (state) {
        case formatstring_state::COPY:
            if (c == '%') {
                state = formatstring_state::SPECIFIER;
            }
            break;
        
        case formatstring_state::SPECIFIER:
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
                state = formatstring_state::SPECIFIER_CUSTOM;
            }
            break;

        case formatstring_state::SPECIFIER_CUSTOM:
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

    template <size_t size>
    consteval formatstring_type_list(const char (&fmtstr)[size]) : type_list(), length(0) {
        size_t index = 0;
        formatstring_state state = formatstring_state::COPY;
        formatstring_modifier modifier = formatstring_modifier::NONE;

        for(auto c: fmtstr) {
            switch (state) {
            case formatstring_state::COPY:
                if (c == '%') {
                    state = formatstring_state::SPECIFIER;
                    modifier = formatstring_modifier::NONE;
                }
                break;
            
            case formatstring_state::SPECIFIER:
                switch (c)
                {
                case '%':
                    state = formatstring_state::COPY;
                    break;
                
                case 'c':
                    switch(modifier) {
                        case formatstring_modifier::NONE:
                            type_list[index++] = type_identifier::int8_t;
                            length += type_length<type_identifier::int8_t>::value;
                            break;
                        case formatstring_modifier::l:
                            type_list[index++] = type_identifier::uint16_t;
                            length += type_length<type_identifier::uint16_t>::value;
                            break;
                    }
                    state = formatstring_state::COPY;
                    break;

                case 'd':
                case 'i':
                    switch (modifier) {
                    case formatstring_modifier::NONE:
                        type_list[index++] = type_identifier::int32_t;
                        length += type_length<type_identifier::int32_t>::value;
                        break;
                    case formatstring_modifier::h:
                        type_list[index++] = type_identifier::int16_t;
                        length += type_length<type_identifier::int16_t>::value;
                        break;
                    case formatstring_modifier::hh:
                        type_list[index++] = type_identifier::int8_t;
                        length += type_length<type_identifier::int8_t>::value;
                        break;
                    case formatstring_modifier::l:
                    case formatstring_modifier::ll:
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
                    switch (modifier) {
                    case formatstring_modifier::NONE:
                        type_list[index++] = type_identifier::uint32_t;
                        length += type_length<type_identifier::uint32_t>::value;
                        break;
                    case formatstring_modifier::h:
                        type_list[index++] = type_identifier::uint16_t;
                        length += type_length<type_identifier::uint16_t>::value;
                        break;
                    case formatstring_modifier::hh:
                        type_list[index++] = type_identifier::uint8_t;
                        length += type_length<type_identifier::uint8_t>::value;
                        break;
                    case formatstring_modifier::l:
                    case formatstring_modifier::ll:
                        type_list[index++] = type_identifier::uint64_t;
                        length += type_length<type_identifier::uint64_t>::value;
                        break;
                    }
                    state = formatstring_state::COPY;
                    break;
                
                case 'f':
                case 'F':
                    switch (modifier)
                    {
                    case formatstring_modifier::h:
                        type_list[index++] = type_identifier::float_t;
                        length += type_length<type_identifier::float_t>::value;
                        break;
                    case formatstring_modifier::NONE:
                        type_list[index++] = type_identifier::double_t;
                        length += type_length<type_identifier::double_t>::value;
                        break;
                    case formatstring_modifier::L:
                        type_list[index++] = type_identifier::longdouble_t;
                        length += type_length<type_identifier::longdouble_t>::value;
                        break;
                    default:
                        type_list[index++] = type_identifier::bad_type;
                        break;
                    }
                    state = formatstring_state::COPY;
                    break;

                case 'v':
                    if(modifier != formatstring_modifier::NONE) {
                        type_list[index++] = type_identifier::bad_type;
                        assert(true);
                        continue;
                    }
                    state = formatstring_state::SPECIFIER_CUSTOM;
                    break;

                // Specifiers
                case 'h':
                    if(modifier != formatstring_modifier::NONE &&
                    modifier != formatstring_modifier::h) {
                        type_list[index++] = type_identifier::bad_type;
                        assert(true);
                        continue;
                    }
                    if (modifier == formatstring_modifier::h)
                        modifier = formatstring_modifier::hh;
                    else modifier = formatstring_modifier::h;
                    break;
                
                case 'l':
                    if(modifier != formatstring_modifier::NONE &&
                       modifier != formatstring_modifier::l) {
                        type_list[index++] = type_identifier::bad_type;
                        assert(true);
                        continue;
                    }
                    if (modifier == formatstring_modifier::l)
                        modifier = formatstring_modifier::ll;
                    else modifier = formatstring_modifier::l;
                    break;

                default:
                    type_list[index++] = type_identifier::bad_type; break;
                } //switch (c)
                
                break; // case formatstring_state::MODIFIER:

            case formatstring_state::SPECIFIER_CUSTOM:
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
consteval size_t check_formatstring_args_internal()
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
consteval size_t check_formatstring_args() {
    if constexpr (COUNT != sizeof...(ARGS)) {
        //std::cout << "COUNT: " << COUNT << ", sizeof...(ARGS): " << sizeof...(ARGS) << std::endl;
        return 0;
    }
    if constexpr (sizeof...(ARGS) >= 1) return check_formatstring_args_internal<COUNT, fmt_list, ARGS...>();
    if constexpr (COUNT >= 1) return 0;
    return SIZE_MAX;
}

constexpr size_t check_success = 0;
constexpr size_t check_args_error = 0x10000;
constexpr size_t check_fmtstr_error = 0x20000;

template <const size_t fmtstr_size, typename... ARGS>
consteval size_t check_formatstring_helper(const char (&fmtstr)[fmtstr_size]) {
    auto type_list = {what_type<ARGS>::value ...};
    formatstring_state state = formatstring_state::COPY;
    formatstring_modifier modifier = formatstring_modifier::NONE;
    auto type_itr = type_list.begin();
    size_t count = 0;

    for(auto c: fmtstr) {
        switch (state) {
        case formatstring_state::COPY:
            if (c == '%') {
                state = formatstring_state::SPECIFIER;
                modifier = formatstring_modifier::NONE;
            }
            break;
        
        case formatstring_state::SPECIFIER:
            if (type_itr == type_list.end()) return check_args_error + count;
            switch (c)
            {
            case '%':
                if (modifier != formatstring_modifier::NONE) return check_fmtstr_error + count;
                state = formatstring_state::COPY;
                break;
            
            case 'c':
                switch(modifier) {
                    case formatstring_modifier::NONE:
                        if (*type_itr != type_identifier::int8_t) return check_args_error + count;
                        break;
                    case formatstring_modifier::l:
                        if (*type_itr != type_identifier::uint16_t) return check_args_error + count;
                        break;
                    default:
                        return check_fmtstr_error + count;
                }
                state = formatstring_state::COPY;
                modifier = formatstring_modifier::NONE;
                ++count;
                type_itr = std::next(type_itr);
                break;

            case 'd':
            case 'i':
                switch (modifier) {
                    case formatstring_modifier::NONE:
                        if (*type_itr != type_identifier::int32_t) return check_args_error + count;
                        break;
                    case formatstring_modifier::h:
                        if (*type_itr != type_identifier::int16_t) return check_args_error + count;
                        break;
                    case formatstring_modifier::hh:
                        if (*type_itr != type_identifier::int8_t) return check_args_error + count;
                        break;
                    case formatstring_modifier::l:
                    case formatstring_modifier::ll:
                        if (*type_itr != type_identifier::int64_t) return check_args_error + count;
                        break;
                    default:
                        return check_fmtstr_error + count;
                }
                state = formatstring_state::COPY;
                modifier = formatstring_modifier::NONE;
                ++count;
                type_itr = std::next(type_itr);
                break;

            case 'u':
            case 'o':
            case 'x':
            case 'X':
                switch (modifier) {
                    case formatstring_modifier::NONE:
                        if (*type_itr != type_identifier::uint32_t) return check_args_error + count;
                        break;
                    case formatstring_modifier::h:
                        if (*type_itr != type_identifier::uint16_t) return check_args_error + count;
                        break;
                    case formatstring_modifier::hh:
                        if (*type_itr != type_identifier::uint8_t) return check_args_error + count;
                        break;
                    case formatstring_modifier::l:
                    case formatstring_modifier::ll:
                        if (*type_itr != type_identifier::uint64_t) return check_args_error + count;
                        break;
                    default:
                        return check_fmtstr_error + count;
                }
                state = formatstring_state::COPY;
                modifier = formatstring_modifier::NONE;
                ++count;
                type_itr = std::next(type_itr);
                break;
            
            case 'f':
            case 'F':
                switch (modifier)
                {
                    case formatstring_modifier::h:
                        if (*type_itr != type_identifier::float_t) return check_args_error + count;
                        break;
                    case formatstring_modifier::NONE:
                        if (*type_itr != type_identifier::double_t) return check_args_error + count;
                        break;
                    case formatstring_modifier::L:
                        if (*type_itr != type_identifier::longdouble_t) return check_args_error + count;
                        break;
                    default:
                        return check_fmtstr_error + count;
                }
                state = formatstring_state::COPY;
                modifier = formatstring_modifier::NONE;
                ++count;
                type_itr = std::next(type_itr);
                break;

            case 'v':
                if(modifier != formatstring_modifier::NONE) return check_fmtstr_error + count;
                state = formatstring_state::SPECIFIER_CUSTOM;
                break;

            // Modifiers
            case 'h':
                switch(modifier) {
                    case formatstring_modifier::NONE:
                        modifier = formatstring_modifier::h;
                        break;
                    case formatstring_modifier::h:
                        modifier = formatstring_modifier::hh;
                        break;
                    default:
                        return check_fmtstr_error + count;
                }
                break;
            
            case 'l':
                switch(modifier) {
                    case formatstring_modifier::NONE:
                        modifier = formatstring_modifier::l;
                        break;
                    case formatstring_modifier::l:
                        modifier = formatstring_modifier::ll;
                        break;
                    default:
                        return check_fmtstr_error + count;
                }
                break;
            
            case 'L':
                if (modifier == formatstring_modifier::NONE)
                    modifier = formatstring_modifier::L;
                else
                    return check_fmtstr_error + count;
                break;

            default:
                return check_fmtstr_error + count;
            } //switch (c)
            
            break; // case formatstring_state::SPECIFIER:

        case formatstring_state::SPECIFIER_CUSTOM:
            switch(c) {
                case 'n':
                case 'N':
                    if (*type_itr != type_identifier::ipv6_socket_addr_t) return check_args_error + count;
                    break;
                case 'i':
                case 'I':
                    if (*type_itr != type_identifier::ipv6_addr_t) return check_args_error + count;
                    break;
                case 'p':
                    if (*type_itr != type_identifier::ipv6_port_t) return check_args_error + count;
                    break;
                case 'e':
                    if (*type_itr != type_identifier::int32_t) return check_args_error + count;
                    break;
                case 'E':
                    if (*type_itr != type_identifier::err_t) return check_args_error + count;
                    break;
                case 'g':
                case 'G':
                    if (*type_itr != type_identifier::guid_t) return check_args_error + count;
                    break;
                case 'v':
                    if (*type_itr != type_identifier::uint32_t) return check_args_error + count;
                    break;
                case 's':
                    if (*type_itr != type_identifier::state_t) return check_args_error + count;
                    break;
                default:
                    return check_fmtstr_error + count;
            }
            state = formatstring_state::COPY;
            ++count;
            type_itr = std::next(type_itr);
            break; // case formatstring_state::SPECIFIER_CUSTOM:
        }
    }

    return check_success;
}

template <const size_t fmtstr_size, const char (&fmtstr)[fmtstr_size], typename... ARGS>
consteval void check_formatstring_assert() {
    constexpr auto ret = check_formatstring_helper<fmtstr_size, ARGS...>(fmtstr);
    static_assert(ret != check_args_error, "Formatted String: bad 1st argument");
    static_assert(ret != check_args_error + 1, "Formatted String: bad 2nd argument");
    static_assert(ret != check_args_error + 2, "Formatted String: bad 3rd argument");
    static_assert(ret != check_args_error + 3, "Formatted String: bad 4th argument");
    static_assert(ret != check_args_error + 4, "Formatted String: bad 5th argument");
    static_assert(ret != check_args_error + 5, "Formatted String: bad 6th argument");
    static_assert(ret != check_args_error + 6, "Formatted String: bad 7th argument");
    static_assert(ret != check_args_error + 7, "Formatted String: bad 8th argument");
    static_assert(ret != check_args_error + 8, "Formatted String: bad 9th argument");
    static_assert(ret != check_args_error + 9, "Formatted String: bad 10th argument");
    static_assert(ret != check_args_error + 10, "Formatted String: bad 11th argument");
    static_assert(ret != check_args_error + 11, "Formatted String: bad 12th argument");
    static_assert(ret != check_args_error + 12, "Formatted String: bad 13th argument");
    static_assert(ret != check_args_error + 13, "Formatted String: bad 14th argument");
    static_assert(ret != check_args_error + 14, "Formatted String: bad 15th argument");
    static_assert(ret != check_args_error + 15, "Formatted String: bad 16th argument");
    static_assert(ret <= check_args_error + 15 || ret >= check_fmtstr_error, "Formatted String: bad argument after 16th argument");

    static_assert(ret != check_fmtstr_error, "Formatted String: bad 1st format specifier in string");
    static_assert(ret != check_fmtstr_error + 1, "Formatted String: bad 2nd format specifier in string");
    static_assert(ret != check_fmtstr_error + 2, "Formatted String: bad 3rd format specifier in string");
    static_assert(ret != check_fmtstr_error + 3, "Formatted String: bad 4th format specifier in string");
    static_assert(ret != check_fmtstr_error + 4, "Formatted String: bad 5th format specifier in string");
    static_assert(ret != check_fmtstr_error + 5, "Formatted String: bad 6th format specifier in string");
    static_assert(ret != check_fmtstr_error + 6, "Formatted String: bad 7th format specifier in string");
    static_assert(ret != check_fmtstr_error + 7, "Formatted String: bad 8th format specifier in string");
    static_assert(ret != check_fmtstr_error + 8, "Formatted String: bad 9th format specifier in string");
    static_assert(ret != check_fmtstr_error + 9, "Formatted String: bad 10th format specifier in string");
    static_assert(ret != check_fmtstr_error + 10, "Formatted String: bad 11th format specifier in string");
    static_assert(ret != check_fmtstr_error + 11, "Formatted String: bad 12th format specifier in string");
    static_assert(ret != check_fmtstr_error + 12, "Formatted String: bad 13th format specifier in string");
    static_assert(ret != check_fmtstr_error + 13, "Formatted String: bad 14th format specifier in string");
    static_assert(ret != check_fmtstr_error + 14, "Formatted String: bad 15th format specifier in string");
    static_assert(ret != check_fmtstr_error + 15, "Formatted String: bad 16th format specifier in string");
    static_assert(ret <= check_fmtstr_error + 15, "Formatted String: bad format specifier in string after 16th specifier");
}

} // namespace rohit