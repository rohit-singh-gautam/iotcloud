////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/varadic.hh>
#include <iot/core/bits.hh>
#include <unistd.h>
#include <iostream>
#include <thread>

namespace rohit {

#define LOGGER_LEVEL_LIST \
    LOGGER_LEVEL_ENTRY(IGNORE) \
    LOGGER_LEVEL_ENTRY(DEBUG) \
    LOGGER_LEVEL_ENTRY(VERBOSE) \
    LOGGER_LEVEL_ENTRY(INFO) \
    LOGGER_LEVEL_ENTRY(WARNING) \
    LOGGER_LEVEL_ENTRY(ERROR) \
    LOGGER_LEVEL_ENTRY(ALERT) \
    LIST_DEFINITION_END

#define LOGGER_MODULE_LIST \
    LOGGER_MODULE_ENTRY(SYSTEM) \
    LOGGER_MODULE_ENTRY(EVENT_DISTRIBUTOR) \
    LOGGER_MODULE_ENTRY(EVENT_EXECUTOR) \
    LOGGER_MODULE_ENTRY(EVENT_SERVER) \
    LOGGER_MODULE_ENTRY(IOT_EVENT_SERVER) \
    LOGGER_MODULE_ENTRY(TEST) \
    LOGGER_MODULE_ENTRY(MAX_MODULE) \
    LIST_DEFINITION_END

#define LOGGER_LOG_LIST \
    \
    LOGGER_ENTRY(END_OF_CLUSTER, IGNORE, SYSTEM, "This is end of cluster only parameter is its length") \
    \
    LOGGER_ENTRY(PTHREAD_CREATE_FAILED, ERROR, SYSTEM, "Unable to create pthread with error %ve") \
    LOGGER_ENTRY(PTHREAD_JOIN_FAILED, WARNING, SYSTEM, "Unable to join pthread with error %ve") \
    LOGGER_ENTRY(EVENT_DIST_CREATE_FAILED, ERROR, EVENT_DISTRIBUTOR, "Event distributor creation failed with error %ve, terminating application") \
    LOGGER_ENTRY(EVENT_DIST_CREATE_NO_THREAD, ERROR, EVENT_DISTRIBUTOR, "Event distributor failed to create any thread, terminating application") \
    LOGGER_ENTRY(EVENT_DIST_LOOP_WAIT_INTERRUPTED, WARNING, EVENT_DISTRIBUTOR, "Event distributor loop interrupted with error %ve,  waiting for a second and retry") \
    LOGGER_ENTRY(EVENT_DIST_TOO_MANY_THREAD, WARNING, EVENT_DISTRIBUTOR, "Event distributor created with threads more than CPUs") \
    LOGGER_ENTRY(EVENT_DIST_EXIT_EPOLL_CLOSE_FAILED, WARNING, EVENT_DISTRIBUTOR, "Event distributor unable to close epoll with error %ve") \
    LOGGER_ENTRY(EVENT_DIST_NO_THREAD_CANCEL, WARNING, EVENT_DISTRIBUTOR, "Event distributor unable to set thread cancel flag with error %ve, exit may not be proper") \
    LOGGER_ENTRY(EVENT_DIST_EXIT_THREAD_CANCEL_FAILED, WARNING, EVENT_DISTRIBUTOR, "Event distributor unable to cancel thread with error %ve") \
    LOGGER_ENTRY(EVENT_DIST_EXIT_THREAD_JOIN_FAILED, WARNING, EVENT_DISTRIBUTOR, "Event distributor unable to join thread with error %ve") \
    LOGGER_ENTRY(EVENT_DIST_CREATE_SUCCESS, VERBOSE, EVENT_DISTRIBUTOR, "Event distributor creation succeeded") \
    LOGGER_ENTRY(EVENT_CREATE_FAILED, ERROR, EVENT_EXECUTOR, "Event creation failed with error %ve") \
    LOGGER_ENTRY(EVENT_CREATE_SUCCESS, VERBOSE, EVENT_EXECUTOR, "Event creation succeeded") \
    LOGGER_ENTRY(EVENT_SERVER_ACCEPT_FAILED, ERROR, EVENT_SERVER, "Event server failed to accept connection with error %ve") \
    LOGGER_ENTRY(IOT_EVENT_SERVER_READ_FAILED, ERROR, IOT_EVENT_SERVER, "IOT Event Server peer read failed with error %ve") \
    \
    LOGGER_ENTRY(SYSTEM_ERROR, ERROR, SYSTEM, "System Error '%ve'") \
    LOGGER_ENTRY(IOT_ERROR, ERROR, SYSTEM, "IOT Error '%vE'") \
    LOGGER_ENTRY(TEST_STATE_LOG, INFO, TEST, "State '%vs'") \
    LOGGER_ENTRY(TEST_GUID_LOG, INFO, TEST, "IOT Error '%vg' caps '%vG'") \
    LOGGER_ENTRY(TEST_FLOAT_LOGS, INFO, TEST, "Test float %%%f, double %lf") \
    LOGGER_ENTRY(TEST_INTEGER_LOGS, INFO, TEST, "Test %%, Integer %i, long %li, long long %lli, Short %hi, Short Short %hhi, Unsigned %u, long %lu, long long %llu, Short %hu, Short Short %hhu") \
    LOGGER_ENTRY(TEST_IPV6ADDR_LOGS, INFO, TEST, "Test char %c, ipv6_socket_addr_t %vn caps: %vN; ipv6_addr_t %vi caps: %vI ipv6_port_t %vp") \
    \
    LOGGER_ENTRY(MAX_LOG, VERBOSE, TEST, "Max log no entry must be made beyond this") \
    LIST_DEFINITION_END

enum class logger_level {
#define LOGGER_LEVEL_ENTRY(x) x,
    LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY
};
class logger_level_operators {
private:
    const logger_level value;

public:
    constexpr logger_level_operators(const logger_level value) : value(value) {}

    constexpr operator const char * () const {
        switch(value) {
#define LOGGER_LEVEL_ENTRY(x) case logger_level::x: return #x;
    LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY            
        }
    }
};

enum class module_t {
#define LOGGER_MODULE_ENTRY(x) x,
    LOGGER_MODULE_LIST
#undef LOGGER_MODULE_ENTRY
};

constexpr size_t bits_to_uint64_array_size(const module_t value) {
    return bits_to_uint64_index(static_cast<size_t>(value)) + 1;
}

constexpr size_t bits_to_uint64_index(const module_t value) {
    return bits_to_uint64_index(static_cast<size_t>(value));
}

constexpr size_t bits_to_uint64_map(const module_t value) {
    return bits_to_uint64_map(static_cast<size_t>(value));
}

enum class log_t : log_id_type {
#define LOGGER_ENTRY(x, y, m, z) x,
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY
};

// This structure is repository of logs and this also helps
// in verification of log entry
template <log_t ID> struct log_description {
    static constexpr const char id_str[] = "BAD_MESSAGE"; \
    static constexpr const char value[] = "BAD Message"; \
    static constexpr const logger_level level = logger_level::VERBOSE; \
    static constexpr const module_t module = module_t::TEST; \
    static constexpr const size_t type_count = 0; \
    static constexpr const auto type_list = formatstring_type_list<type_count>(""); \
    static constexpr const auto length = 0; \
    log_description() {  } \
};

#define LOGGER_ENTRY(x, y, m, z) template <> struct log_description<log_t::x> { \
        static constexpr const char id_str[] = #x; \
        static constexpr const char value[] = z; \
        static constexpr const logger_level level = logger_level::y; \
        static constexpr const module_t module = module_t::m; \
        static constexpr const size_t type_count = formatstring_count(z); \
        static constexpr const auto type_list = formatstring_type_list<type_count>(z); \
        static constexpr const auto length = type_list.length; \
        log_description() { static_assert(type_list.type_list_check(), "Bad log string"); } \
    };
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY

inline constexpr const char * get_log_description(log_t id) {
    switch (id) {
        default: // This will avoid error, such condition will never reach
            assert(true);
#define LOGGER_ENTRY(x, y, m, z) case log_t::x: return z;
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY
    }
}

inline constexpr size_t get_log_length(log_t id) {
    switch (id) {
        default: // This will avoid error, such condition will never reach
            assert(true);
#define LOGGER_ENTRY(x, y, m, z) case log_t::x: return log_description<log_t::x>::length;
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY
    }
}

inline constexpr const char * get_log_id_string(log_t id) {
    switch (id) {
        default: // This will avoid error, such condition will never reach
            assert(true);
#define LOGGER_ENTRY(x, y, m, z) case log_t::x: return #x;
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY
    }
}

#define LOGGER_ENTRY(x, y, m, z) +1
constexpr size_t log_t_count = 0 + LOGGER_LOG_LIST;
#undef LOGGER_ENTRY

template <log_t ID, typename... ARGS>
inline constexpr size_t check_formatstring_args() {
    return check_formatstring_args<log_description<ID>::type_count, log_description<ID>::type_list, ARGS...>();
}

template <log_t ID, typename... ARGS>
inline constexpr size_t check_formatstring_args(const ARGS&...) {
    return check_formatstring_args<log_description<ID>::type_count, log_description<ID>::type_list, ARGS...>();
}

class logger_logs_entry_common {
public:
    static const constexpr size_t max_size = 240;

public:
    const log_t id;

    inline constexpr logger_logs_entry_common(const log_t id) : id(id) {}
} __attribute__((packed));

class logger_logs_entry_end_of_cluster : public logger_logs_entry_common {
public:
    const uint8_t length;
    inline constexpr logger_logs_entry_end_of_cluster(uint8_t length)
        : logger_logs_entry_common(log_t::END_OF_CLUSTER), length(length) {}
} __attribute__((packed));

template <log_t ID, typename... ARGS>
class logger_logs_entry : private logger_logs_entry_common {
public:
    static const constexpr size_t argsize = sizeofvaradic<ARGS...>::size;
    static const constexpr size_t totalsize = sizeof(logger_logs_entry<ID,ARGS...>);

private:
    uint8_t arguments[argsize];

public:
    inline constexpr logger_logs_entry(const ARGS&... args)
            : logger_logs_entry_common(ID) {
        static_assert(totalsize != log_description<ID>::length, "Total size of argument must be less than 256");
        static_assert(log_description<ID>::type_count == sizeof...(ARGS), "Wrong number of parameters");
        static_assert(check_formatstring_args<ID, ARGS...>() == SIZE_MAX, "Wrong parameter type");
        copyvaradic(arguments, args...);
    }
} __attribute__((packed));

template <log_t ID>
class logger_logs_entry<ID> : public logger_logs_entry_common {
public:
    inline constexpr logger_logs_entry()
            : logger_logs_entry_common(ID) { }
} __attribute__((packed));

class logger_logs_entry_read : public logger_logs_entry_common {
public:
    static const constexpr size_t argsize = logger_logs_entry_common::max_size - sizeof(logger_logs_entry_common);
    const uint8_t arguments[argsize];

    constexpr logger_level level() const {
        switch(id) {
        default: // This will avoid error, such condition will never reach
            assert(true);
#define LOGGER_ENTRY(x, y, m, z) case log_t::x: return logger_level::y;
            LOGGER_LOG_LIST
#undef LOGGER_ENTRY           
        }
    }

    // This class does not have constructor as this will be alway mapped to memory
} __attribute__((packed));

class log_cluster_entry {
public:
    static constexpr size_t max_cluster_size = 8192;
    bool used;
    size_t start;
    size_t index;
    uint8_t buffer[max_cluster_size];

    constexpr log_cluster_entry()
        : used(false), start(0), index(0), buffer() { }
};

class log_buffer {
public:
    static constexpr size_t max_cluster_count = 2;

private:
    size_t current_cluster = 0;
    log_cluster_entry cluster_list[max_cluster_count];

public:
    constexpr log_buffer() : current_cluster(0), cluster_list() { }

    constexpr log_cluster_entry *get_current_cluster() {
        return cluster_list + current_cluster;
    }

    // For timebeing no check will be performed
    constexpr void next_cluster() {
        ++current_cluster;
        if (current_cluster == max_cluster_count) current_cluster = 0;
        log_cluster_entry &log_cluster = *get_current_cluster();
        log_cluster.index = 0;
        log_cluster.start = 0;
    }
};

// This is global
// Any prameter change will have global impact
class logger {
private:
    // Global parameter
    // Only one log allowed hence static
    static int file_descriptor;
    void write(const void *data, const size_t length);

    log_buffer log_buf;

    friend class logreader;

public:
    static void init(const std::string &filename);
    void flush();

    static constexpr const char * id_string(const log_t id) {
        return get_log_id_string(id);
    }

    static constexpr const char * id_description(const log_t id) {
        return get_log_description(id);
    }

    template <log_t ID>
    static constexpr const char * id_description() {
        return log_description<ID>::value;
    }

    // Supported format specifier
    // %d or %i - signed integer (int32_t)
    // %u - unsigned integer (uint32_t)
    // %o - unsigned integer (uint32_t)(Octal)
    // %x - unsigned integer (uint32_t)(hex small case)
    // %X - unsigned integer (uint32_t)(hex capital case)
    // %f - floating point lower case (float)
    // %F - floating point upper case (float)
    // %c - character (char)
    // %v - Custom
    //      %vn: IPv6 network Address format
    //      %vN: IPv6 network Address format in caps
    //      %vi: IPv6 address
    //      %vi: IPv6 address in caps
    //      %vp: IPv6 port
    //      %ve: System errno
    //      %vE: IOT error
    //      %vg: GUID lower case
    //      %vG: GUID upper case
    //      %vs: State of an execution
    // %% - %
    //
    // Supported format length
    // h - short - 16 bits
    // hh - ultra short 8 bits
    // l - long
    // ll - long long
    // z - size_t
    template <log_t ID, typename... ARGS>
    void log(const ARGS&... args)
    {
        logger_logs_entry<ID, ARGS...> logs_entry(args...);
        write(&logs_entry, sizeof(logs_entry));
    }

}; // class logger

extern rohit::logger glog;

// This is not global
// No need to write very optimise reader
class logreader {
private:
    int file_descriptor;
    char text[1024];

    static const constexpr size_t args_size = logger_logs_entry_common::max_size - sizeof(logger_logs_entry_common);
    uint8_t data_args[args_size];

public:
    logreader(const std::string &filename);

    const std::string readnext();

}; // class logreader


} // namespace rohit