////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/pthread_helper.hh>
#include <iot/core/varadic.hh>
#include <iot/core/bits.hh>
#include <iot/core/config.hh>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <queue>
#include <algorithm>

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
    LOGGER_MODULE_ENTRY(IOT_HTTPSERVER) \
    LOGGER_MODULE_ENTRY(IOT_HTTP2SERVER) \
    LOGGER_MODULE_ENTRY(TEST) \
    LOGGER_MODULE_ENTRY(MAX_MODULE) \
    LOGGER_MODULE_ENTRY(UNKNOWN) \
    LIST_DEFINITION_END

#define LOGGER_LOG_LIST \
    LOGGER_ENTRY(SEGMENTATION_FAULT, ALERT, SYSTEM, "Segmentation fault occurred !!!!!!!!!!!!!!") \
    \
    LOGGER_ENTRY(PTHREAD_CREATE_FAILED, ERROR, SYSTEM, "Unable to create pthread with error %ve") \
    LOGGER_ENTRY(PTHREAD_JOIN_FAILED, WARNING, SYSTEM, "Unable to join pthread with error %ve") \
    LOGGER_ENTRY(SOCKET_CREATE_SUCCESS, DEBUG, SOCKET, "Socket %i created") \
    LOGGER_ENTRY(SOCKET_CREATE_FAILED, INFO, SOCKET, "Socket creation failed with error %ve") \
    LOGGER_ENTRY(SOCKET_CLOSE_SUCCESS, DEBUG, SOCKET, "Socket %i closed") \
    LOGGER_ENTRY(SOCKET_CLOSE_FAILED, INFO, SOCKET, "Socket %i close failed with error %ve") \
    LOGGER_ENTRY(SOCKET_BIND_SUCCESS, DEBUG, SOCKET, "Socket %i, port %i bind success") \
    LOGGER_ENTRY(SOCKET_LISTEN_SUCCESS, DEBUG, SOCKET, "Socket %i, port %i listen success") \
    LOGGER_ENTRY(SOCKET_ACCEPT_SUCCESS, DEBUG, SOCKET, "Socket %i accept success, new socket created %i") \
    LOGGER_ENTRY(SOCKET_SET_NONBLOCKING_FAILED, ERROR, SOCKET, "Socket %i setting non blocking failed") \
    \
    LOGGER_ENTRY(SOCKET_SSL_INITIALIZE, INFO, SOCKET, "Socket initialize SSL") \
    LOGGER_ENTRY(SOCKET_SSL_INITIALIZE_ATTEMPT, DEBUG, SOCKET, "Socket initialize SSL attempt  %i") \
    LOGGER_ENTRY(SOCKET_SSL_CERT_LOAD_SUCCESS, INFO, SOCKET, "Loaded SSL certificate") \
    LOGGER_ENTRY(SOCKET_SSL_CERT_LOAD_FAILED, ERROR, SOCKET, "Unable to load SSL certificate, exiting") \
    LOGGER_ENTRY(SOCKET_SSL_PRIKEY_LOAD_SUCCESS, INFO, SOCKET, "Loaded Primary Key") \
    LOGGER_ENTRY(SOCKET_SSL_PRIKEY_LOAD_FAILED, ERROR, SOCKET, "Unable to load Primary Key, exiting") \
    LOGGER_ENTRY(SOCKET_SSL_CLEANUP, INFO, SOCKET, "Socket cleanup SSL") \
    LOGGER_ENTRY(SOCKET_SSL_CLEANUP_ATTEMPT, DEBUG, SOCKET, "Socket cleanup SSL left %i") \
    LOGGER_ENTRY(SOCKET_SSL_ACCEPT_RETRY, DEBUG, SOCKET, "SSL Socket %i accept retry SSL Accept") \
    LOGGER_ENTRY(SOCKET_SSL_ACCEPT_SUCCESS, VERBOSE, SOCKET, "SSL Socket %i accept success") \
    LOGGER_ENTRY(SOCKET_SSL_ACCEPT_FAILED, ERROR, SOCKET, "SSL Socket %i accept failed") \
    \
    LOGGER_ENTRY(EVENT_DIST_CREATING_THREAD, DEBUG, EVENT_DISTRIBUTOR, "Event distributor creating %llu threads") \
    LOGGER_ENTRY(EVENT_DIST_LOOP_CREATED, DEBUG, EVENT_DISTRIBUTOR, "Event distributor thread loop created") \
    LOGGER_ENTRY(EVENT_DIST_CREATE_FAILED, ERROR, EVENT_DISTRIBUTOR, "Event distributor creation failed with error %ve, terminating application") \
    LOGGER_ENTRY(EVENT_DIST_CREATE_NO_THREAD, ERROR, EVENT_DISTRIBUTOR, "Event distributor failed to create any thread, terminating application") \
    LOGGER_ENTRY(EVENT_DIST_LOOP_WAIT_INTERRUPTED, WARNING, EVENT_DISTRIBUTOR, "Event distributor loop interrupted with error %ve,  waiting for a second and retry") \
    LOGGER_ENTRY(EVENT_DIST_TOO_MANY_THREAD, WARNING, EVENT_DISTRIBUTOR, "Event distributor created with threads more than CPUs") \
    LOGGER_ENTRY(EVENT_DIST_EXIT_EPOLL_CLOSE_FAILED, WARNING, EVENT_DISTRIBUTOR, "Event distributor unable to close epoll with error %ve") \
    LOGGER_ENTRY(EVENT_DIST_EXIT_THREAD_JOIN_FAILED, WARNING, EVENT_DISTRIBUTOR, "Event distributor unable to join thread with error %ve") \
    LOGGER_ENTRY(EVENT_DIST_EXIT_THREAD_JOIN_SUCCESS, VERBOSE, EVENT_DISTRIBUTOR, "Event distributor join thread success") \
    LOGGER_ENTRY(EVENT_DIST_CREATE_SUCCESS, INFO, EVENT_DISTRIBUTOR, "Event distributor creation succeeded") \
    LOGGER_ENTRY(EVENT_DIST_TERMINATING, INFO, EVENT_DISTRIBUTOR, "Event distributor TERMINATING") \
    LOGGER_ENTRY(EVENT_DIST_EVENT_RECEIVED, DEBUG, EVENT_DISTRIBUTOR, "Event distributor event %vv receive") \
    LOGGER_ENTRY(EVENT_DIST_DEADLOCK_DETECTED, ALERT, EVENT_DISTRIBUTOR, "Event distributor deadlock detected in thread %llu, state %vs") \
    LOGGER_ENTRY(EVENT_DIST_PAUSED_THREAD, DEBUG, EVENT_DISTRIBUTOR, "Event distributor pausing thread %llu") \
    LOGGER_ENTRY(EVENT_DIST_RESUMED_THREAD, DEBUG, EVENT_DISTRIBUTOR, "Event distributor resumed thread %llu") \
    LOGGER_ENTRY(EVENT_DIST_PAUSED_THREAD_FAILED, ERROR, EVENT_DISTRIBUTOR, "Event distributor failed to pause") \
    \
    LOGGER_ENTRY(EVENT_CREATE_FAILED, ERROR, EVENT_EXECUTOR, "Event creation failed with error %ve") \
    LOGGER_ENTRY(EVENT_CREATE_SUCCESS, DEBUG, EVENT_EXECUTOR, "Event creation succeeded") \
    LOGGER_ENTRY(EVENT_REMOVE_SUCCESS, DEBUG, EVENT_EXECUTOR, "Event removal succeeded") \
    LOGGER_ENTRY(EVENT_REMOVE_FAILED, INFO, EVENT_EXECUTOR, "Event removal failed with error %ve") \
    \
    LOGGER_ENTRY(EVENT_SERVER_RECEIVED_CLOSED, INFO, EVENT_SERVER, "Event server listener close request received") \
    LOGGER_ENTRY(EVENT_SERVER_RECEIVED_EVENT, DEBUG, EVENT_SERVER, "Event server with ID %i received event") \
    LOGGER_ENTRY(EVENT_SERVER_SSL_RECEIVED_EVENT, DEBUG, EVENT_SERVER, "SSL Event server with ID %i received event %vv") \
    LOGGER_ENTRY(EVENT_SERVER_ACCEPT_FAILED, ERROR, EVENT_SERVER, "Event server failed to accept connection with error %ve") \
    LOGGER_ENTRY(EVENT_SERVER_SSL_ACCEPT_FAILED, ERROR, EVENT_SERVER, "SSL Event server failed to accept connection with error %ve") \
    LOGGER_ENTRY(EVENT_SERVER_PEER_CREATED, VERBOSE, EVENT_SERVER, "Event server new peer requested from remote %vN") \
    LOGGER_ENTRY(EVENT_SERVER_SSL_PEER_CREATED, VERBOSE, EVENT_SERVER, "SSL Event server new peer requested from remote %vN") \
    LOGGER_ENTRY(EVENT_SERVER_HELPER_CREATE_FAILED, ERROR, EVENT_SERVER, "Event server helper create failed") \
    LOGGER_ENTRY(EVENT_SERVER_HELPER_WRITE_FAILED, WARNING, EVENT_SERVER, "Event server event write failed with error %ve") \
    LOGGER_ENTRY(EVENT_SERVER_HELPER_READ_FAILED, WARNING, EVENT_SERVER, "Event server event read failed with error %ve") \
    LOGGER_ENTRY(EVENT_SERVER_HELPER_UNKNOWN, WARNING, EVENT_SERVER, "Event server unknown message") \
    LOGGER_ENTRY(EVENT_SERVER_MOVED_ENTERED, WARNING, EVENT_SERVER, "Entered event server after move, must not happen") \
    LOGGER_ENTRY(EVENT_SERVER_UNKNOWN_STATE, WARNING, EVENT_SERVER, "Entered event server for unknown state %vs") \
    \
    LOGGER_ENTRY(IOT_EVENT_SERVER_COMMAND_RECEIVED, VERBOSE, IOT_EVENT_SERVER, "IOT Event Server received message %vN") \
    LOGGER_ENTRY(IOT_EVENT_SERVER_READ_FAILED, DEBUG, IOT_EVENT_SERVER, "IOT Event Server peer read failed with error %vE") \
    LOGGER_ENTRY(IOT_EVENT_SERVER_CONNECTION_CLOSED, INFO, IOT_EVENT_SERVER, "IOT Event Server connection closed fd %i") \
    LOGGER_ENTRY(IOT_EVENT_SERVER_WRITE_FAILED, ERROR, IOT_EVENT_SERVER, "IOT Event Server peer write failed with error %vE") \
    LOGGER_ENTRY(IOT_EVENT_SERVER_NULL_SOCKET, ERROR, IOT_EVENT_SERVER, "IOT Event Server command received with null socket") \
    \
    LOGGER_ENTRY(FILEWATCHER_EVENT_CREATE_FAILED, ERROR, IOT_HTTPSERVER, "FILEWATCHER event create failed") \
    LOGGER_ENTRY(FILEWATCHER_ADD_FOLDER_FAILED, WARNING, IOT_HTTPSERVER, "FILEWATCHER failed to add folder for watch with error %ve") \
    LOGGER_ENTRY(FILEWATCHER_EVENT_READ_FAILED, INFO, IOT_HTTPSERVER, "FILEWATCHER event read failed with error %ve") \
    LOGGER_ENTRY(HTTP_EVENT_SERVER_READ_FAILED, DEBUG, IOT_HTTPSERVER, "HTTP Event Server peer read failed with error %vE") \
    \
    LOGGER_ENTRY(HTTP2_EVENT_SERVER_READ_FAILED, DEBUG, IOT_HTTP2SERVER, "HTTP2 Event Server peer read failed with error %vE") \
    \
    LOGGER_ENTRY(SYSTEM_ERROR, ERROR, SYSTEM, "System Error '%ve'") \
    LOGGER_ENTRY(IOT_ERROR, ERROR, SYSTEM, "IOT Error '%vE'") \
    LOGGER_ENTRY(TEST_STATE_LOG, INFO, TEST, "State '%vs'") \
    LOGGER_ENTRY(TEST_GUID_LOG, INFO, TEST, "IOT Error '%vg' caps '%vG'") \
    LOGGER_ENTRY(TEST_FLOAT_LOGS, INFO, TEST, "Test float %%%hf, double %f") \
    LOGGER_ENTRY(TEST_INTEGER_LOGS, INFO, TEST, "Test %%, Integer %i, long %li, long long %lli, Short %hi, Short Short %hhi, Unsigned %u, long %lu, long long %llu, Short %hu, Short Short %hhu") \
    LOGGER_ENTRY(TEST_IPV6ADDR_LOGS, INFO, TEST, "Test char %c, ipv6_socket_addr_t %vn caps: %vN; ipv6_addr_t %vi caps: %vI ipv6_port_t %vp") \
    \
    LOGGER_ENTRY(WEB_SERVER_NO_EXTENSION, DEBUG, SYSTEM, "Web Server, file without extension is not supported, ignoring") \
    LOGGER_ENTRY(WEB_SERVER_UNSUPPORTED_EXTENSION, DEBUG, SYSTEM, "Web Server, unsupported file extension, ignoring") \
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

inline module_t to_module_t(const std::string module_name) {
    static std::unordered_map<std::string, module_t> module_enum = {
#define LOGGER_MODULE_ENTRY(x) {#x, module_t::x},
        LOGGER_MODULE_LIST
#undef LOGGER_MODULE_ENTRY
    };

    auto module_itr = module_enum.find(module_name);
    if (module_itr == module_enum.end()) {
        return module_t::UNKNOWN;
    }

    return module_itr->second;
}

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

constexpr const char * get_log_description(log_t id) {
    switch (id) {
        default: // This will avoid error, such condition will never reach
            assert(true);
#define LOGGER_ENTRY(x, y, m, z) case log_t::x: return z;
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY
    }
}

constexpr size_t get_log_length(log_t id) {
    switch (id) {
        default: // This will avoid error, such condition will never reach
            assert(true);
#define LOGGER_ENTRY(x, y, m, z) case log_t::x: return log_description<log_t::x>::length;
    LOGGER_LOG_LIST
#undef LOGGER_ENTRY
    }
}

constexpr const char * get_log_id_string(log_t id) {
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
consteval void check_formatstring_args() {
    check_formatstring_assert<std::size(log_description<ID>::value), log_description<ID>::value, ARGS...>();
}


class logger_logs_entry_common {
public:
    static const constexpr size_t max_size = 240;

public:
    const int64_t timestamp;
    const log_t id;

    constexpr logger_logs_entry_common(
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
    constexpr logger_logs_entry(const int64_t timestamp, const ARGS&... args)
            : logger_logs_entry_common(timestamp, ID) {
        static_assert(totalsize != log_description<ID>::length, "Total size of argument must be less than 256");
        static_assert(log_description<ID>::type_count == sizeof...(ARGS), "Wrong number of parameters");
        check_formatstring_args<ID, ARGS...>();
        copyvaradic(arguments, args...);
    }
} __attribute__((packed));

template <log_t ID>
class logger_logs_entry<ID> : public logger_logs_entry_common {
public:
    constexpr logger_logs_entry(const int64_t timestamp)
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

class active_module {
private:
    std::bitset<(size_t)module_t::MAX_MODULE> enabled_module_bits;

public:
    constexpr active_module() : enabled_module_bits() {}

    inline bool add_module(const std::string module_name) {
        return add_module(to_module_t(module_name));
    }

    constexpr bool add_module(const module_t module) {
        if (module == module_t::UNKNOWN) return false;
        enabled_module_bits.set((size_t)module);
        return true;
    }

    inline bool remove_module(const std::string module_name) {
        return remove_module(to_module_t(module_name));
    }

    constexpr bool remove_module(const module_t module) {
        if (module == module_t::UNKNOWN) return false;
        enabled_module_bits.reset((size_t)module);
        return true;
    }

    inline void enable_all() {
        enabled_module_bits.set();
    }

    inline void clear_all() {
        enabled_module_bits.reset();
    }

    template <log_t ID>
    constexpr bool is_enabled() {
        // ERROR and ALERT for all modules always enabled
        return log_description<ID>::level >= logger_level::ERROR ||
            enabled_module_bits.test((size_t)log_description<ID>::module);
    }

}; // class active_module

extern active_module enabled_log_module;

class logger;

class logger_list {
    // As this will be in global variable,
    // enabled would be accessed even after destruction
    // mutex and logger_store cannot be accessed after
    // destruction of this class
    // Creation of thread is a slow process we do not have
    // to be super optimize here
    std::atomic<bool> enabled = true;
    std::mutex mutex { };
    std::unordered_set<logger *> logger_store { }; 

    int fd = 0;

public:
    ~logger_list();

    void add(logger *new_logger);
    void remove(logger *new_logger);

    void flush();

    void set_fd(const int fd);
};

// This is global
// Any parameter change will have global impact
class logger {
public:
    static constexpr size_t max_log_memory = 1_mb;
    static constexpr std::chrono::milliseconds wait_for_free = std::chrono::milliseconds(1);

private:
    size_t next_write; // Index were write will start
    size_t next_read; // Index where read will start

    uint8_t mem_buffer[max_log_memory];

public:
    logger() : next_write(0), next_read(0), mem_buffer() {
        logger::all.add(this);
    }
    ~logger() {
        logger::all.remove(this);
    }

    void flush(const int filedescriptor) {
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
    }

    template <log_t ID, typename... ARGS>
    void log(const ARGS&... args)
    {
        if (!enabled_log_module.is_enabled<ID>()) return;
        const int64_t nanosecond = std::chrono::system_clock::now().time_since_epoch().count();
        logger_logs_entry<ID, ARGS...> logs_entry(nanosecond, args...);

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
                    if (next_read != next_write) std::this_thread::sleep_for(wait_for_free);
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
    }

    static logger_list all;

}; // class logger

// This is multi threaded
extern thread_local logger _log;

template <log_t ID, typename... ARGS>
void log(const ARGS&... args) {
    _log.log<ID, ARGS...>(args...);
}

void init_log_thread(const char *filename);
void destroy_log_thread();
void segv_log_flush();


class logger_logs_entry_read_compare {
public:
    inline bool operator() (logger_logs_entry_read *lhs, logger_logs_entry_read *rhs) {
        return lhs->timestamp > rhs->timestamp;
    }
};

// This is not global
// No need to write very optimise reader
class logreader {
private:
    const int file_descriptor;
    char text[1024];

    std::priority_queue<
        logger_logs_entry_read *, 
        std::vector<logger_logs_entry_read *>,
        logger_logs_entry_read_compare> priqueue;

    uint64_t last_read_time = 0;
    static constexpr int64_t log_thread_wait_in_millis = 50;
    static constexpr uint64_t buffer_time_in_nanos = config::log_thread_wait_in_millis * 4 * 1000000;

public:
    logreader(const std::string &filename);

    // This is blocking call
    logger_logs_entry_read *readnext();

    // This is blocking call
    const std::string readnextstring(bool live = true);

}; // class logreader


} // namespace rohit