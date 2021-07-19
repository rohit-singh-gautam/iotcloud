////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include "types.hh"
#include <errno.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>

namespace rohit {

#define ERROR_T_LIST \
    ERROR_T_ENTRY(SUCCESS, "SUCCESS") \
    ERROR_T_ENTRY(SUCCESS_NONBLOCKING, "Call is non blocking") \
    ERROR_T_ENTRY(SOCKET_CONNECT_ALREADY_CONNECTED, "Socket is already connected") \
    ERROR_T_ENTRY(GENERAL_FAILURE, "Failed") \
    ERROR_T_ENTRY(BIND_FAILURE, "Unable to bind to a socket") \
    ERROR_T_ENTRY(LISTEN_FAILURE, "Socket is already connected") \
    ERROR_T_ENTRY(ACCEPT_FAILURE, "Unable to accept connection to a socket") \
    ERROR_T_ENTRY(CLOSE_FAILURE, "Unable to close a socket") \
    ERROR_T_ENTRY(RECEIVE_FAILURE, "Unable to read from a socket") \
    ERROR_T_ENTRY(SEND_FAILURE, "Unable to write to a socket") \
    \
    ERROR_T_ENTRY(PTHREAD_JOIN_FAILURE, "Unable to join to a thread") \
    ERROR_T_ENTRY(PTHREAD_JOIN_DEADLOCK_FAILURE, "pthread_join created a deadlock") \
    ERROR_T_ENTRY(PTHREAD_JOIN_NOT_JOINABLE_FAILURE, "Either thread is not joinable probably other thread is trying to join") \
    ERROR_T_ENTRY(PTHREAD_JOIN_INVALID_THREAD_ID_FAILURE, "No thread is running with given thread ID") \
    \
    ERROR_T_ENTRY(PTHREAD_CREATE_FAILURE, "Unable to create thread") \
    \
    ERROR_T_ENTRY(GUID_BAD_STRING_FAILURE, "Bad GUID string") \
    \
    ERROR_T_ENTRY(MESSAGE_COMMAND_LIMIT_FAILURE, "Cannot add more command to message") \
    \
    ERROR_T_ENTRY(SOCKET_FAILURE, "Unable to create socket") \
    ERROR_T_ENTRY(SOCKET_PERMISSION_FAILURE, "Insufficient permission to create socket") \
    ERROR_T_ENTRY(SOCKET_ADDRESS_NOT_SUPPORTED, "Address type is not supported") \
    ERROR_T_ENTRY(SOCKET_PROTOCOL_NOT_SUPPORTED, "Protocol type is not supported") \
    ERROR_T_ENTRY(SOCKET_LIMIT_REACHED, "Filedescriptor limit for socket reached") \
    ERROR_T_ENTRY(SOCKET_INSUFFICIENT_MEMORY, "Insufficient memory") \
    \
    ERROR_T_ENTRY(SOCKET_CONNECT_FAILURE, "Unable to connect to socket") \
    ERROR_T_ENTRY(SOCKET_CONNECT_PERMISSION_FAILURE, "Insufficient permission to connect to socket") \
    ERROR_T_ENTRY(SOCKET_CONNECT_ADDRESS_IN_USE, "Already connected with this address") \
    ERROR_T_ENTRY(SOCKET_CONNECT_ADDRESS_NOT_SUPPORTED, "Address type is not supported") \
    ERROR_T_ENTRY(SOCKET_CONNECT_INVALID_ID, "Invalid socket ID") \
    ERROR_T_ENTRY(SOCKET_CONNECT_CONNECTION_REFUSED, "Connection refused, remote server not accepting connection") \
    ERROR_T_ENTRY(SOCKET_CONNECT_INTERRUPTED, "Connect was interrupted due to signal") \
    ERROR_T_ENTRY(SOCKET_CONNECT_NETWORK_UNREACHABLE, "Unreachable server") \
    ERROR_T_ENTRY(SOCKET_CONNECT_UNSUPPORTED_PROTOCOL, "Unsupported protocol") \
    ERROR_T_ENTRY(SOCKET_CONNECT_TIMEOUT, "Unable to connect as it timeout") \
    \
    ERROR_T_ENTRY(SOCKET_SSL_CONTEXT_FAILED, "Creation on SSL context failed") \
    ERROR_T_ENTRY(SOCKET_SSL_CERTIFICATE_FAILED, "Failed to load SSL certificate") \
    ERROR_T_ENTRY(SOCKET_SSL_PRIKEY_FAILED, "Failed to load Primay Key") \
    \
    ERROR_T_ENTRY(SOCKOPT_FAILURE, "Unable to set socket option") \
    ERROR_T_ENTRY(SOCKOPT_BAD_ID, "Unable to set socket option, bad socket ID") \
    ERROR_T_ENTRY(SOCKOPT_UNKNOWN_OPTION, "Unknown socket option") \
    \
    ERROR_T_ENTRY(LOG_READ_FAILURE, "Unable to read log") \
    ERROR_T_ENTRY(LOG_UNSUPPORTED_TYPE_FAILURE, "Unsupported log type") \
    ERROR_T_ENTRY(LOG_FILE_OPEN_FAILURE, "Unable to open log file") \
    \
    ERROR_T_ENTRY(MATH_INSUFFICIENT_BUFFER, "Buffer is not sufficient to store result, partial and wrong result may have been written to buffer") \
    \
    ERROR_T_ENTRY(EVENT_DIST_CREATE_FAILED, "Event distributor creation failed") \
    ERROR_T_ENTRY(EVENT_CREATE_FAILED, "Event creation failed") \
    ERROR_T_ENTRY(EVENT_REMOVE_FAILED, "Event remove failed") \
    \
    ERROR_T_ENTRY(SSL_CONNECT_FAILED, "Failed to create SSL session") \
    ERROR_T_ENTRY(CRYPTO_UNKNOWN_ALGORITHM, "Unknown crypto algorithm") \
    ERROR_T_ENTRY(CRYPTO_MEMORY_FAILURE, "Failed to allocated OpenSSL memory") \
    ERROR_T_ENTRY(CRYPTO_CREATE_CONTEXT_FAILED, "Failed to create OpenSSL encryption/decryption context") \
    ERROR_T_ENTRY(CRYPTO_INIT_AES_FAILED, "Failed to initialize OpenSSL AES encryption/decryption") \
    ERROR_T_ENTRY(CRYPTO_ENCRYPT_AES_FAILED, "Failed OpenSSL AES encryption") \
    ERROR_T_ENTRY(CRYPTO_DECRYPT_AES_FAILED, "Failed OpenSSL AES decryption") \
    \
    ERROR_T_ENTRY(MAX_FAILURE, "Max failure nothing beyond this") \
    LIST_DEFINITION_END

enum class err_t : log_id_type {
#define ERROR_T_ENTRY(x, y) x,
        ERROR_T_LIST
#undef ERROR_T_ENTRY
};

constexpr err_t operator++(err_t &err) { return err = static_cast<err_t>(static_cast<log_id_type>(err) + 1); }
constexpr err_t operator++(err_t err, int) { 
    err_t reterr = err;
    err = static_cast<err_t>(static_cast<log_id_type>(err) + 1);
    return reterr;
}

template <bool null_terminated = true>
constexpr size_t to_string_size(const err_t &val) {
    if constexpr (null_terminated) {
        constexpr size_t displaystr_size[] =  {
#define ERROR_T_ENTRY(x, y) sizeof(#x" - " y),
    ERROR_T_LIST
#undef ERROR_T_ENTRY
        };
        return displaystr_size[(size_t)val];
    } else {
        constexpr size_t displaystr_size[] =  {
#define ERROR_T_ENTRY(x, y) (sizeof(#x" - " y) - 1),
    ERROR_T_LIST
#undef ERROR_T_ENTRY
        };
        return displaystr_size[(size_t)val];
    }
}

constexpr bool isFailure(const err_t &err) {
    return err != err_t::SUCCESS;
}

constexpr const char *err_t_string[] = {
#define ERROR_T_ENTRY(x, y) {#x" - " y},
    ERROR_T_LIST
#undef ERROR_T_ENTRY
};

template <bool null_terminated = true>
constexpr size_t to_string(const err_t &val, char *dest) {
    auto len = to_string_size<null_terminated>(val);
    const char *errstr = err_t_string[(size_t)val];
    std::copy(errstr, errstr + len, dest);
    return len;
}

class error_c {
public:

private:
    err_t value;

public:
    inline error_c(const error_c &err) : value(err.value) {}
    inline error_c(const err_t value) : value(value) {}

    inline error_c& operator=(const error_c rhs) { value = rhs.value; return *this; }

    // Description will not be compared
    inline bool operator==(const error_c rhs) const { return value == rhs; }
    inline bool operator!=(const error_c rhs) const { return value != rhs; }
    inline bool operator==(const err_t rhs) const { return value == rhs; }
    inline bool operator!=(const err_t rhs) const { return value != rhs; }

    inline const std::string to_string() const {
        return std::string(err_t_string[(size_t)value]);
    }
    inline operator const std::string() const { return to_string(); }

    constexpr operator err_t() const { return value; }
    constexpr operator log_id_type() const { return static_cast<log_id_type>(value); }


    inline bool isSuccess() const { return value == err_t::SUCCESS; }
    inline bool isFailure() const { return value != err_t::SUCCESS; }

    static inline err_t socket_create_ret() {
        switch (errno)
        {
        case 0: return err_t::SUCCESS;
        case EACCES: return err_t::SOCKET_PERMISSION_FAILURE;
        case EAFNOSUPPORT: return err_t::SOCKET_ADDRESS_NOT_SUPPORTED;
        case EPROTONOSUPPORT:
        case EINVAL: return err_t::SOCKET_PROTOCOL_NOT_SUPPORTED;
        case EMFILE: return err_t::SOCKET_LIMIT_REACHED;
        case ENOBUFS:
        case ENOMEM: return err_t::SOCKET_INSUFFICIENT_MEMORY;
        default: return err_t::SOCKET_FAILURE;
        }
    }

    static inline err_t socket_connect_ret() {
        switch (errno)
        {
        case 0: return err_t::SUCCESS;
        case EAGAIN:
        case EALREADY:
        case EINPROGRESS: return err_t::SUCCESS_NONBLOCKING;
        case EISCONN: return err_t::SOCKET_CONNECT_ALREADY_CONNECTED;
        case EACCES:
        case EPERM: return err_t::SOCKET_CONNECT_PERMISSION_FAILURE;
        case EADDRINUSE: 
        case EADDRNOTAVAIL: return err_t::SOCKET_CONNECT_ADDRESS_IN_USE;
        case EAFNOSUPPORT: return err_t::SOCKET_CONNECT_ADDRESS_NOT_SUPPORTED;
        case EBADF: 
        case EFAULT:
        case ENOTSOCK: return err_t::SOCKET_CONNECT_INVALID_ID;
        case ECONNREFUSED: return err_t::SOCKET_CONNECT_CONNECTION_REFUSED;
        case EINTR: return err_t::SOCKET_CONNECT_INTERRUPTED;
        case ENETUNREACH: return err_t::SOCKET_CONNECT_NETWORK_UNREACHABLE;
        case EPROTOTYPE: return err_t::SOCKET_CONNECT_UNSUPPORTED_PROTOCOL;
        case ETIMEDOUT: return err_t::SOCKET_CONNECT_TIMEOUT;

        default: return err_t::SOCKET_CONNECT_FAILURE;
        }
    }

    static inline err_t sockopt_ret() {
        switch (errno) {
            case 0: return err_t::SUCCESS;
            case EBADF:
            case ENOTSOCK: return err_t::SOCKOPT_BAD_ID;
            case ENOPROTOOPT: return err_t::SOCKOPT_UNKNOWN_OPTION;
            case EINVAL:
            default: return err_t::SOCKOPT_FAILURE;
        }
    }

}; // class err_t

inline std::ostream& operator<<(std::ostream& os, const err_t &error) {
    char str[to_string_size(error)] = {};
    to_string(error, str);
    return os << str;
}

class exception_t : public error_c {
public:
    using error_c::error_c;

};

inline std::ostream& operator<<(std::ostream& os, const exception_t &error) {
    char str[to_string_size(error)] = {};
    to_string(error, str);
    return os << str;
}



} // namespace rohit