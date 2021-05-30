#pragma once
#include "core/varadic.hh"
#include <unistd.h>
#include <iostream>
#include <thread>

namespace rohit {

class log_str {
public:
    const char * id_str;
    const char * description;

    inline log_str(const char *errStr, const char * desc) : id_str(errStr), description(desc) {}
};

#define LOGGER_LOG_LIST \
    LOGGER_ENTRY(PTHREAD_CREATE_FAILED, "Unable to create pthread") \
    LOGGER_ENTRY(PTHREAD_JOIN_FAILED, "Unable to join pthread %lu") \
    LOGGER_ENTRY(SYSTEM_ERROR, "System Error '%ve'") \
    LOGGER_ENTRY(IOT_ERROR, "IOT Error '%vE'") \
    LOGGER_ENTRY(TEST_STATE_LOG, "State '%vs'") \
    LOGGER_ENTRY(TEST_GUID_LOG, "IOT Error '%vg' caps '%vG'") \
    LOGGER_ENTRY(TEST_FLOAT_LOGS, "Test float %%%f, double %lf") \
    LOGGER_ENTRY(TEST_INTEGER_LOGS, "Test %%, Integer %i, long %li, long long %lli, Short %hi, Short Short %hhi, Unsigned %u, long %lu, long long %llu, Short %hu, Short Short %hhu") \
    LOGGER_ENTRY(TEST_IPV6ADDR_LOGS, "Test char %c, ipv6_socket_addr_t %vn caps: %vN; ipv6_addr_t %vi caps: %vI ipv6_port_t %vp") \
    \
    LOGGER_ENTRY(MAX_LOG, "Max log no entry must be made beyond this") \
    LIST_DEFINITION_END

#define LOGGER_LEVEL_LIST \
    LOGGER_LEVEL_ENTRY(VERBOSE) \
    LOGGER_LEVEL_ENTRY(INFO) \
    LOGGER_LEVEL_ENTRY(WARNING) \
    LOGGER_LEVEL_ENTRY(ERROR) \
    LIST_DEFINITION_END

enum class logger_level : uint8_t {
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

enum class logger_type : uint8_t {
    HEADER,
    END_OF_CLUSTER,
    TIMEBASE,
    LOGS,
};

enum class logger_message_id : log_id_type {
#define LOGGER_ENTRY(x, y) x,
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY
};

template <logger_message_id ID> struct log_description { static constexpr char const value[] = "BAD Message"; };
#define LOGGER_ENTRY(x, y) template <> struct log_description<logger_message_id::x> { \
        static constexpr const char id_str[] = #x; \
        static constexpr const char value[] = y; \
        static constexpr const size_t type_count = formatstring_count(y); \
        static constexpr const auto type_list = formatstring_type_list<type_count>(y); \
        static constexpr const auto length = type_list.length; \
        log_description<logger_message_id::x>() { static_assert(type_list.type_list_check(), "Bad log string"); } \
    };
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY

inline constexpr const char * get_log_description(logger_message_id id) {
    switch (id) {
        default: // This will avoid error, such condition will never reach
            assert(true);
#define LOGGER_ENTRY(x, y) case logger_message_id::x: return log_description<logger_message_id::x>::value;
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY
    }
}

inline constexpr size_t get_log_length(logger_message_id id) {
    switch (id) {
        default: // This will avoid error, such condition will never reach
            assert(true);
#define LOGGER_ENTRY(x, y) case logger_message_id::x: return log_description<logger_message_id::x>::length;
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY
    }
}

inline constexpr const char * get_log_id_string(logger_message_id id) {
    switch (id) {
        default: // This will avoid error, such condition will never reach
            assert(true);
#define LOGGER_ENTRY(x, y) case logger_message_id::x: return log_description<logger_message_id::x>::id_str;
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY
    }
}

#define LOGGER_ENTRY(x, y) +1
constexpr size_t logger_message_id_count = 0 + LOGGER_LOG_LIST;
#undef LOGGER_ENTRY

template <logger_message_id ID, typename... ARGS>
inline constexpr size_t check_formatstring_args() {
    return check_formatstring_args<log_description<ID>::type_count, log_description<ID>::type_list, ARGS...>();
}

template <logger_message_id ID, typename... ARGS>
inline constexpr size_t check_formatstring_args(const ARGS&...) {
    return check_formatstring_args<log_description<ID>::type_count, log_description<ID>::type_list, ARGS...>();
}

class logger_logs_entry_header {
public:
    const logger_type       log_type:6; 
    const logger_level      level:2;

    inline constexpr logger_logs_entry_header(
        const logger_type log_type, const logger_level level)
        : log_type(log_type), level(level) {}
} __attribute__((packed));

class logger_logs_entry_end_of_cluster : public logger_logs_entry_header {
public:
    const uint8_t length;
    inline constexpr logger_logs_entry_end_of_cluster(uint8_t length)
        : logger_logs_entry_header(logger_type::END_OF_CLUSTER, logger_level::VERBOSE), length(length) {}
} __attribute__((packed));

class logger_logs_entry_common : public logger_logs_entry_header {
public:
    static const constexpr size_t max_size = 240;

public:
    const logger_message_id id;

    inline constexpr logger_logs_entry_common(
        const logger_level      level,
        const logger_message_id id)
            : logger_logs_entry_header(logger_type::LOGS, level), id(id) {}

} __attribute__((packed));

template <logger_level LEVEL, logger_message_id ID, typename... ARGS>
class logger_logs_entry : private logger_logs_entry_common {
public:
    static const constexpr size_t argsize = sizeofvaradic<ARGS...>::size;
    static const constexpr size_t totalsize = sizeof(logger_logs_entry<LEVEL,ID,ARGS...>);

private:
    uint8_t arguments[argsize];

public:
    inline constexpr logger_logs_entry(const ARGS&... args)
            : logger_logs_entry_common(LEVEL, ID) {
        static_assert(totalsize != log_description<ID>::length, "Total size of argument must be less than 256");
        static_assert(log_description<ID>::type_count == sizeof...(ARGS), "Wrong number of parameters");
        static_assert(check_formatstring_args<ID, ARGS...>() == SIZE_MAX, "Wrong parameter type");
        copyvaradic(arguments, args...);
    }
} __attribute__((packed));

template <logger_level LEVEL, logger_message_id ID>
class logger_logs_entry<LEVEL, ID> : public logger_logs_entry_common {
public:
    inline constexpr logger_logs_entry()
            : logger_logs_entry_common(LEVEL, ID) { }
} __attribute__((packed));

class logger_logs_entry_read : public logger_logs_entry_common {
public:
    static const constexpr size_t argsize = logger_logs_entry_common::max_size - sizeof(logger_logs_entry_common);
    const uint8_t arguments[argsize];

    // This class does not have constructor as this will be alway mapped to memory
} __attribute__((packed));

// This is global
// Any prameter change will have global impact
class logger {
private:
    static int file_descriptor;

    static void write(const void *data, const size_t length);

    template <logger_level LEVEL, logger_message_id ID, typename... ARGS>
    static void log(const ARGS&... args)
    {
        logger_logs_entry<LEVEL, ID, ARGS...> logs_entry(args...);
        write(&logs_entry, sizeof(logs_entry));
    }

    friend class logreader;
    
    template<logger_message_id ID, typename... ARGS>
    friend constexpr void log_verbose(const ARGS&... args);

    template<logger_message_id ID, typename... ARGS>
    friend constexpr void log_info(const ARGS&... args);

    template<logger_message_id ID, typename... ARGS>
    friend constexpr void log_warning(const ARGS&... args);

    template<logger_message_id ID, typename... ARGS>
    friend constexpr void log_error(const ARGS&... args);

public:
    static void init(const std::string &filename);
    static void flush();

    static constexpr const char * id_string(const logger_message_id id) {
        return get_log_id_string(id);
    }

    static constexpr const char * id_description(const logger_message_id id) {
        return get_log_description(id);
    }

    template <logger_message_id ID>
    static constexpr const char * id_description() {
        return log_description<ID>::value;
    }

}; // class logger

template<logger_message_id ID, typename... ARGS>
constexpr void log_verbose(const ARGS&... args) {
    logger::log<logger_level::VERBOSE, ID, ARGS...>(args...);
}

template<logger_message_id ID, typename... ARGS>
constexpr void log_info(const ARGS&... args) {
    logger::log<logger_level::INFO, ID, ARGS...>(args...);
}

template<logger_message_id ID, typename... ARGS>
constexpr void log_warning(const ARGS&... args) {
    logger::log<logger_level::WARNING, ID, ARGS...>(args...);
}

template<logger_message_id ID, typename... ARGS>
constexpr void log_error(const ARGS&... args) {
    logger::log<logger_level::ERROR, ID, ARGS...>(args...);
}


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