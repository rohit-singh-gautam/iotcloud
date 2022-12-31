/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// This program is free software: you can redistribute it and/or modify it under the terms //
// of the GNU General Public License as published by the Free Software Foundation, either  //
// version 3 of the License, or (at your option) any later version.                        //
//                                                                                         //
// This program is distributed in the hope that it will be useful, but WITHOUT ANY         //
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A         //
// PARTICULAR PURPOSE. See the GNU General Public License for more details.                //
//                                                                                         //
// You should have received a copy of the GNU General Public License along with this       //
// program. If not, see <https://www.gnu.org/licenses/>.                                   //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/log_entry.hh>
#include <iot/core/pthread_helper.hh>
#include <iot/core/varadic.hh>
#include <iot/core/bits.hh>
#include <iot/core/config.hh>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <unordered_set>
#include <bitset>
#include <queue>
#include <algorithm>
#include <filesystem>

namespace rohit {

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
    logger_level module_level[static_cast<size_t>(module_t::MAX_MODULE)] { logger_level::ERROR };

public:
    constexpr active_module() {}

    void set_module(const module_t module, const logger_level level);

    void enable_all(const logger_level level = logger_level::DEBUG);

    inline void clear_all() {
        enable_all(logger_level::ERROR);
    }

    template <log_t ID>
    constexpr inline bool is_enabled() {
        // ERROR and ALERT for all modules always enabled
        return log_description<ID>::level >=
            module_level[static_cast<size_t>(log_description<ID>::module)];
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
    void flush(logger *);

    void set_fd(const int fd);
};

// This is global
// Any parameter change will have global impact
class logger {
public:
    static constexpr size_t init_log_memory = 1_mb;
    static constexpr size_t max_log_memory = 1_mb;
    static constexpr std::chrono::milliseconds wait_for_free = std::chrono::milliseconds(1);

private:
    uint8_t* next_write = nullptr;
    uint8_t* next_read = nullptr;    

    size_t buffer_size = 0;
    uint8_t* buffer = nullptr;
    uint8_t* buffer_end = nullptr;

    std::mutex mutex;

public:
    logger();
    ~logger();

    // This will flush only
    // Will not reallocate memory
    void flush(const int fd);
    void replinish();

    template <log_t ID, typename... ARGS>
    inline void log(const ARGS&... args)
    {
        if constexpr (log_description<ID>::level < logger_level::ERROR) {
            if (!enabled_log_module.is_enabled<ID>()) return;
        }
        const int64_t nanosecond = std::chrono::system_clock::now().time_since_epoch().count();
        logger_logs_entry<ID, ARGS...> logs_entry(nanosecond, args...);

        auto new_next_write = next_write + sizeof(logs_entry);
        if (new_next_write >= buffer_end) [[unlikely]] {
            replinish();
            new_next_write = next_write + sizeof(logs_entry);
        }

        std::copy(
                (uint8_t *)&logs_entry,
                (uint8_t *)&logs_entry + sizeof(logs_entry),
                next_write);

        next_write = new_next_write;        
    }

    static logger_list all;

}; // class logger

// This is multi threaded
extern thread_local logger _log;

template <log_t ID, typename... ARGS>
void log(const ARGS&... args) {
    _log.log<ID, ARGS...>(args...);
}

void init_log_thread(const std::filesystem::path &filename);
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
    static constexpr int64_t buffer_time_in_nanos = config::log_thread_wait_in_millis * 4 * 1000000;

public:
    logreader(const std::string &filename);

    // This is blocking call
    logger_logs_entry_read *readnext();

    // This is blocking call
    const std::string readnextstring(bool live = true);

}; // class logreader


} // namespace rohit