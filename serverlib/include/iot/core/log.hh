////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/varadic.hh>
#include <iot/core/bits.hh>
#include <iot/core/config.hh>
#include <unistd.h>
#include <iostream>
#include <thread>

namespace rohit {

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
    //      %vn: IPv6 socket Address format
    //      %vN: IPv6 socket Address format in caps
    //      %vi: IPv6 address
    //      %vi: IPv6 address in caps
    //      %vp: IPv6 port
    //      %ve: System errno
    //      %vE: IOT error
    //      %vg: GUID lower case
    //      %vG: GUID upper case
    //      %vv: Epoll event
    //      %vs: State of an execution
    // %% - %
    //
    // Supported format length
    // h - short - 16 bits
    // hh - ultra short 8 bits
    // l - long
    // ll - long long

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
    LOGGER_MODULE_ENTRY(SOCKET) \
    LOGGER_MODULE_ENTRY(EVENT_DISTRIBUTOR) \
    LOGGER_MODULE_ENTRY(EVENT_EXECUTOR) \
    LOGGER_MODULE_ENTRY(EVENT_SERVER) \
    LOGGER_MODULE_ENTRY(IOT_EVENT_SERVER) \
    LOGGER_MODULE_ENTRY(TEST) \
    LOGGER_MODULE_ENTRY(MAX_MODULE) \
    LIST_DEFINITION_END

#define LOGGER_LOG_LIST \
    LOGGER_ENTRY(PTHREAD_CREATE_FAILED, ERROR, SYSTEM, "Unable to create pthread with error %ve") \
    LOGGER_ENTRY(PTHREAD_JOIN_FAILED, WARNING, SYSTEM, "Unable to join pthread with error %ve") \
    LOGGER_ENTRY(SOCKET_CREATE_SUCCESS, DEBUG, SOCKET, "Socket %i created") \
    LOGGER_ENTRY(SOCKET_CREATE_FAILED, DEBUG, SOCKET, "Socket creation failed with error %ve") \
    LOGGER_ENTRY(SOCKET_CLOSE_SUCCESS, DEBUG, SOCKET, "Socket %i closed") \
    LOGGER_ENTRY(SOCKET_CLOSE_FAILED, DEBUG, SOCKET, "Socket %i close failed with error %ve") \
    LOGGER_ENTRY(SOCKET_BIND_SUCCESS, DEBUG, SOCKET, "Socket %i, port %i bind success") \
    LOGGER_ENTRY(SOCKET_LISTEN_SUCCESS, DEBUG, SOCKET, "Socket %i, port %i listen success") \
    LOGGER_ENTRY(SOCKET_ACCEPT_SUCCESS, DEBUG, SOCKET, "Socket %i accept success, new socket created %i") \
    LOGGER_ENTRY(EVENT_DIST_CREATING_THREAD, DEBUG, EVENT_DISTRIBUTOR, "Event distributor creating %llu threads") \
    LOGGER_ENTRY(EVENT_DIST_LOOP_CREATED, DEBUG, EVENT_DISTRIBUTOR, "Event distributor thread loop created") \
    LOGGER_ENTRY(EVENT_DIST_CREATE_FAILED, ERROR, EVENT_DISTRIBUTOR, "Event distributor creation failed with error %ve, terminating application") \
    LOGGER_ENTRY(EVENT_DIST_CREATE_NO_THREAD, ERROR, EVENT_DISTRIBUTOR, "Event distributor failed to create any thread, terminating application") \
    LOGGER_ENTRY(EVENT_DIST_LOOP_WAIT_INTERRUPTED, WARNING, EVENT_DISTRIBUTOR, "Event distributor loop interrupted with error %ve,  waiting for a second and retry") \
    LOGGER_ENTRY(EVENT_DIST_TOO_MANY_THREAD, WARNING, EVENT_DISTRIBUTOR, "Event distributor created with threads more than CPUs") \
    LOGGER_ENTRY(EVENT_DIST_EXIT_EPOLL_CLOSE_FAILED, WARNING, EVENT_DISTRIBUTOR, "Event distributor unable to close epoll with error %ve") \
    LOGGER_ENTRY(EVENT_DIST_NO_THREAD_CANCEL, WARNING, EVENT_DISTRIBUTOR, "Event distributor unable to set thread cancel flag with error %ve, exit may not be proper") \
    LOGGER_ENTRY(EVENT_DIST_EXIT_THREAD_CANCEL_FAILED, WARNING, EVENT_DISTRIBUTOR, "Event distributor unable to cancel thread with error %ve") \
    LOGGER_ENTRY(EVENT_DIST_EXIT_THREAD_JOIN_FAILED, WARNING, EVENT_DISTRIBUTOR, "Event distributor unable to join thread with error %ve") \
    LOGGER_ENTRY(EVENT_DIST_CREATE_SUCCESS, INFO, EVENT_DISTRIBUTOR, "Event distributor creation succeeded") \
    LOGGER_ENTRY(EVENT_DIST_TERMINATING, INFO, EVENT_DISTRIBUTOR, "Event distributor TERMINATING") \
    LOGGER_ENTRY(EVENT_DIST_EVENT_RECEIVED, DEBUG, EVENT_DISTRIBUTOR, "Event distributor event %vv receive") \
    \
    LOGGER_ENTRY(EVENT_CREATE_FAILED, ERROR, EVENT_EXECUTOR, "Event creation failed with error %ve") \
    LOGGER_ENTRY(EVENT_CREATE_SUCCESS, VERBOSE, EVENT_EXECUTOR, "Event creation succeeded") \
    \
    LOGGER_ENTRY(EVENT_SERVER_RECEIVED_EVENT, DEBUG, EVENT_SERVER, "Event server with ID %i received event %vv") \
    LOGGER_ENTRY(EVENT_SERVER_ACCEPT_FAILED, ERROR, EVENT_SERVER, "Event server failed to accept connection with error %ve") \
    LOGGER_ENTRY(EVENT_SERVER_PEER_CREATED, VERBOSE, EVENT_SERVER, "Event server new peer requested from remote %vN") \
    \
    LOGGER_ENTRY(IOT_EVENT_SERVER_COMMAND_RECEIVED, VERBOSE, IOT_EVENT_SERVER, "IOT Event Server received message %vN") \
    LOGGER_ENTRY(IOT_EVENT_SERVER_READ_FAILED, ERROR, IOT_EVENT_SERVER, "IOT Event Server peer read failed with error %ve") \
    LOGGER_ENTRY(IOT_EVENT_SERVER_CONNECTION_CLOSED, INFO, IOT_EVENT_SERVER, "IOT Event Server connection closed fd %i") \
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
    const int64_t timestamp;
    const log_t id;

    inline constexpr logger_logs_entry_common(
        const int64_t timestamp,
        const log_t id) : timestamp(timestamp), id(id) {}
} __attribute__((packed));

template <log_t ID, typename... ARGS>
class logger_logs_entry : public logger_logs_entry_common {
public:
    static const constexpr size_t argsize = sizeofvaradic<ARGS...>::size;
    static const constexpr size_t totalsize = sizeof(logger_logs_entry<ID,ARGS...>);

private:
    uint8_t arguments[argsize];

public:
    inline constexpr logger_logs_entry(const int64_t timestamp, const ARGS&... args)
            : logger_logs_entry_common(timestamp, ID) {
        static_assert(totalsize != log_description<ID>::length, "Total size of argument must be less than 256");
        static_assert(log_description<ID>::type_count == sizeof...(ARGS), "Wrong number of parameters");
        static_assert(check_formatstring_args<ID, ARGS...>() == SIZE_MAX, "Wrong parameter type");
        copyvaradic(arguments, args...);
    }
} __attribute__((packed));

template <log_t ID>
class logger_logs_entry<ID> : public logger_logs_entry_common {
public:
    inline constexpr logger_logs_entry(const int64_t timestamp)
            : logger_logs_entry_common(timestamp, ID) { }
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

template <bool multi_thread>
class logger_thread {
public:
    inline void lock() {}
    inline void unlock() {}
};

template <>
class logger_thread<true> {
public:
    pthread_mutex_t _lock;

    inline logger_thread() {
        pthread_mutex_init(&_lock, nullptr);
    }

    inline void lock() {
        pthread_mutex_lock(&_lock);
    }
    inline void unlock() {
        pthread_mutex_unlock(&_lock);
    }
};

// This is global
// Any prameter change will have global impact
template <bool multi_thread>
class logger : public logger_thread<multi_thread> {
public:
    static constexpr size_t max_log_memory = 1_mb;
    static constexpr std::chrono::milliseconds wait_for_free = std::chrono::milliseconds(1);

private:
    size_t next_write; // Index were write will start
    size_t next_read; // Index where read will start

    uint8_t mem_buffer[max_log_memory];

    void lock() { logger_thread<multi_thread>::lock(); }
    void unlock() { logger_thread<multi_thread>::unlock(); }

public:
    logger() : next_write(0), next_read(0), mem_buffer() {
        logger_add(this);
    }
    
    static void flushall(const int filedescriptor) {
        for(size_t index = 0; index < logger_count; ++index) {
            logger_array[index]->flush(filedescriptor);
        }
    }

    void flush(const int filedescriptor) {
        lock();

        size_t new_next_write = next_write;
        
        if (new_next_write > next_read) {
            size_t data_size = new_next_write - next_read;
            auto ret = ::write(
                filedescriptor,
                mem_buffer + next_read,
                data_size);
            if constexpr (config::debug) {
                if (ret < 0) {
                    // Log to console
                    std::cerr << "Failed to write log with error: " << errno << "\n";
                }
            }
            next_read = new_next_write;
        } else if (new_next_write < next_read) {
            size_t first_size = max_log_memory - next_read;
            // second_size = new_next_write;
            auto ret = ::write(
                filedescriptor,
                mem_buffer + next_read,
                first_size);
            if constexpr (config::debug) {
                if (ret < 0) {
                    // Log to console
                    std::cerr << "Failed to write log with error: " << errno << "\n";
                }
            }

            ret = ::write(filedescriptor, mem_buffer, new_next_write);
            if constexpr (config::debug) {
                if (ret < 0) {
                    // Log to console
                    std::cerr << "Failed to write log with error: " << errno << "\n";
                }
            }

            next_read = new_next_write;
        }

        sync();
        unlock();
    }

    template <log_t ID, typename... ARGS>
    void log(const ARGS&... args)
    {
        const int64_t nanosecond = std::chrono::system_clock::now().time_since_epoch().count();
        logger_logs_entry<ID, ARGS...> logs_entry(nanosecond, args...);
        lock();

        if constexpr (config::log_with_check || config::debug) {
            while (true) {
                size_t new_next_write = next_write + sizeof(logs_entry);
                if (next_write >= next_read) {
                    // ....ddddd....
                    // Data is in between mem_buffer

                    if (new_next_write < max_log_memory) {
                        // Just write the data
                        std::copy(
                            (uint8_t *)&logs_entry,
                            (uint8_t *)&logs_entry + sizeof(logs_entry),
                            mem_buffer + next_write);
                        next_write = new_next_write;
                        break;
                    } else {
                        new_next_write -= max_log_memory;
                        if (new_next_write < next_read) {
                            size_t first_write = max_log_memory - next_write;
                            // second_write = new_next_write;
                            std::copy(
                            (uint8_t *)&logs_entry,
                            (uint8_t *)&logs_entry + first_write,
                            mem_buffer + next_write);

                            std::copy(
                            (uint8_t *)&logs_entry + first_write,
                            (uint8_t *)&logs_entry + sizeof(logs_entry),
                            mem_buffer);

                            next_write = new_next_write;
                            break;
                        }
                    }

                } else {
                    // dd.......dddd
                    // Data is at start and end of mem_buffer
                    if (new_next_write < next_read) {
                        std::copy(
                            (uint8_t *)&logs_entry,
                            (uint8_t *)&logs_entry + sizeof(logs_entry),
                            mem_buffer + next_write);
                        next_write = new_next_write;
                        break;
                    }
                }

                if constexpr (!config::log_with_check) {
                    assert(true);
                } else {
                    // Unlock required before sleep
                    // As thread that write to file
                    unlock();
                    if (next_read != next_write) std::this_thread::sleep_for(wait_for_free);
                    lock();
                }
            }
        } else {
            // There will be no check
            // This is achieved by having huge memory for logs
            // 1MB of memory can hold more that 60000 logs

            size_t new_next_write = next_write + sizeof(logs_entry);
            if (new_next_write < max_log_memory) {
                std::copy(
                        (uint8_t *)&logs_entry,
                        (uint8_t *)&logs_entry + sizeof(logs_entry),
                        mem_buffer + next_write);
                    next_write = new_next_write;
            } else {
                size_t first_write = max_log_memory - next_write;
                // second_write = new_next_write;

                std::copy(
                (uint8_t *)&logs_entry,
                (uint8_t *)&logs_entry + first_write,
                mem_buffer + next_write);

                std::copy(
                (uint8_t *)&logs_entry + first_write,
                (uint8_t *)&logs_entry + sizeof(logs_entry),
                mem_buffer);

                next_write = new_next_write;
            }
        }

        unlock();
    }

    // Maximum 128 thread supported
    static constexpr size_t max_logger = multi_thread ? 1 : 128;
    static size_t logger_count;
    static logger *logger_array[max_logger];

    static void logger_add(logger *new_logger) {
        assert(logger_count < max_logger);
        logger_array[logger_count++] = new_logger;
    }

}; // class logger

// This is multi threaded
extern logger<true> glog;

void init_log_thread(const char *filename);
void destroy_log_thread();

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