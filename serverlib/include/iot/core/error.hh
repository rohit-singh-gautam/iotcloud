#pragma once

#include "types.hh"
#include <errno.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace rohit {

class error_str {
public:
    const char * error;
    const char * generic_description;

    inline error_str(const char *errStr, const char * desc) : error(errStr), generic_description(desc) {}
};

#define ERROR_T_DEFINITION_END

#define ERROR_T_SUCCESSTYPE \
    ERROR_T_ENTRY(SUCCESS, "SUCCESS") \
    ERROR_T_ENTRY(SUCCESS_NONBLOCKING, "Call is non blocking") \
    ERROR_T_ENTRY(SOCKET_CONNECT_ALREADY_CONNECTED, "Socket is already connected") \
    ERROR_T_DEFINITION_END

#define ERROR_T_FAILURETYPE \
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
    ERROR_T_ENTRY(PTHREAD_CREATE_RESOURCE_FAILURE, "Insufficient resource or /proc/sys/kernel/pid_max limit reached") \
    ERROR_T_ENTRY(PTHREAD_CREATE_INVALID_ATTRIBUTE_FAILURE, "Invalid pthread attributes passed") \
    ERROR_T_ENTRY(PTHREAD_CREATE_PERMISSION_FAILURE, "No permission to create thread") \
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
    ERROR_T_ENTRY(SOCKOPT_FAILURE, "Unable to set socket option") \
    ERROR_T_ENTRY(SOCKOPT_BAD_ID, "Unable to set socket option, bad socket ID") \
    ERROR_T_ENTRY(SOCKOPT_UNKNOWN_OPTION, "Unknown socket option") \
    \
    ERROR_T_ENTRY(LOG_READ_FAILURE, "Unable to read log") \
    ERROR_T_ENTRY(LOG_UNSUPPORTED_TYPE_FAILURE, "Unsupported log type") \
    ERROR_T_ENTRY(LOG_FILE_OPEN_FAILURE, "Unable to open log file") \
    \
    ERROR_T_ENTRY(MATH_INSUFFICIENT_BUFFER, "Buffer is not sufficient to store result, partial and wrong result may have been written to buffer") \
    ERROR_T_DEFINITION_END

class error_t {
public:
    enum error_internal_t : log_id_type {
#define ERROR_T_ENTRY(x, y) x,
        ERROR_T_SUCCESSTYPE
        ERROR_T_FAILURETYPE
#undef ERROR_T_ENTRY

        MAX_FAILURE
    };

private:
    log_id_type value;
    static const error_str displayString[];

public:
    inline error_t(const error_t &err) : value(err.value) {}
    inline error_t(const error_internal_t value) : value(value) {}

    inline error_t& operator=(const error_t rhs) { value = rhs.value; return *this; }

    // Description will not be compared
    inline bool operator==(const error_t rhs) const { return value == rhs.value; }
    inline bool operator!=(const error_t rhs) const { return value != rhs.value; }
    inline bool operator==(const error_internal_t rhs) const { return value == rhs; }
    inline bool operator!=(const error_internal_t rhs) const { return value != rhs; }

    inline const std::string to_string() const {
        auto dispStr = displayString[value];
        return std::string(dispStr.error) + " - " + dispStr.generic_description;
    }
    inline operator const std::string() const { return to_string(); }

    inline operator error_internal_t() const { return static_cast<error_internal_t>(value); }

    inline bool isSuccess() const { return value == SUCCESS; }
    inline bool isFailure() const { return value != SUCCESS; }

    static inline error_t pthread_join_ret(const int retval) {
        switch (retval) {
            case 0: return SUCCESS;
            case EDEADLK: return PTHREAD_JOIN_DEADLOCK_FAILURE;
            case EINVAL: return PTHREAD_JOIN_NOT_JOINABLE_FAILURE;
            case ESRCH: return PTHREAD_JOIN_INVALID_THREAD_ID_FAILURE;
            default: return PTHREAD_JOIN_FAILURE;
        }
    }

    static inline error_t pthread_create_ret(const int retval) {
        switch (retval) {
            case 0: return SUCCESS;
            case EAGAIN: return PTHREAD_CREATE_RESOURCE_FAILURE;
            case EINVAL: return PTHREAD_CREATE_INVALID_ATTRIBUTE_FAILURE;
            case EPERM: return PTHREAD_CREATE_PERMISSION_FAILURE;
            default: return PTHREAD_CREATE_FAILURE;
        }
    }

    static inline error_t socket_create_ret() {
        switch (errno)
        {
        case 0: return SUCCESS;
        case EACCES: return SOCKET_PERMISSION_FAILURE;
        case EAFNOSUPPORT: return SOCKET_ADDRESS_NOT_SUPPORTED;
        case EPROTONOSUPPORT:
        case EINVAL: return SOCKET_PROTOCOL_NOT_SUPPORTED;
        case EMFILE: return SOCKET_LIMIT_REACHED;
        case ENOBUFS:
        case ENOMEM: return SOCKET_INSUFFICIENT_MEMORY;
        default: return SOCKET_FAILURE;
        }
    }

    static inline error_t socket_connect_ret() {
        switch (errno)
        {
        case 0: return SUCCESS;
        case EAGAIN:
        case EALREADY:
        case EINPROGRESS: return SUCCESS_NONBLOCKING;
        case EISCONN: return SOCKET_CONNECT_ALREADY_CONNECTED;
        case EACCES:
        case EPERM: return SOCKET_CONNECT_PERMISSION_FAILURE;
        case EADDRINUSE: 
        case EADDRNOTAVAIL: return SOCKET_CONNECT_ADDRESS_IN_USE;
        case EAFNOSUPPORT: return SOCKET_CONNECT_ADDRESS_NOT_SUPPORTED;
        case EBADF: 
        case EFAULT:
        case ENOTSOCK: return SOCKET_CONNECT_INVALID_ID;
        case ECONNREFUSED: return SOCKET_CONNECT_CONNECTION_REFUSED;
        case EINTR: return SOCKET_CONNECT_INTERRUPTED;
        case ENETUNREACH: return SOCKET_CONNECT_NETWORK_UNREACHABLE;
        case EPROTOTYPE: return SOCKET_CONNECT_UNSUPPORTED_PROTOCOL;
        case ETIMEDOUT: return SOCKET_CONNECT_TIMEOUT;

        default: return SOCKET_CONNECT_FAILURE;
        }
    }

    static inline error_t sockopt_ret() {
        switch (errno) {
            case 0: return SUCCESS;
            case EBADF:
            case ENOTSOCK: return SOCKOPT_BAD_ID;
            case ENOPROTOOPT: return SOCKOPT_UNKNOWN_OPTION;
            case EINVAL:
            default: return error_t::SOCKOPT_FAILURE;
        }
    }

}; // class error_t

inline std::ostream& operator<<(std::ostream& os, const error_t &error) {
    return os << error.to_string();
}

class exception_t : public error_t {
public:
    using error_t::error_t;

};

inline std::ostream& operator<<(std::ostream& os, const exception_t &error) {
    return os << error.to_string();
}



} // namespace rohit