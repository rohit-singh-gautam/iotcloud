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
#include <unordered_map>
#include <string>
#include <cassert>

namespace rohit {

#define LIST_DEFINITION_END

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
    //      %vc: SSL error
    //      %vm: Module name
    //      %vl: Log level
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
    LOGGER_MODULE_ENTRY(CONFIG_SERVER) \
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
    LOGGER_ENTRY(SYSTEM_ERROR, ERROR, SYSTEM, "System Error '%ve'") \
    LOGGER_ENTRY(IOT_ERROR, ERROR, SYSTEM, "IOT Error '%vE'") \
    LOGGER_ENTRY(SETTING_LOG_LEVEL_FAILED, ALERT, SYSTEM, "FAILED: Setting Log level %vl for module %vm") \
    LOGGER_ENTRY(SETTING_LOG_LEVEL, ALERT, SYSTEM, "Setting Log level %vl for module %vm") \
    LOGGER_ENTRY(SETTING_LOG_LEVEL_ALL, ALERT, SYSTEM, "Setting Log level %vl for all modules") \
    \
    LOGGER_ENTRY(CONFIG_SERVER_INIT_SUCCESS, ALERT, EVENT_SERVER, "Config Server: Started Successfully. Number of Message: %li, Message Size: %li") \
    LOGGER_ENTRY(CONFIG_SERVER_INIT_FAILED_RETRY, WARNING, EVENT_SERVER, "Config Server: initialize failed, retrying... Error: %ve") \
    LOGGER_ENTRY(CONFIG_SERVER_INIT_FAILED, ERROR, EVENT_SERVER, "Config Server: initialize failed with error: %ve") \
    LOGGER_ENTRY(CONFIG_SERVER_READ_FAILED, ERROR, EVENT_SERVER, "Config Server: read failed, stopping server. Error: %ve") \
    LOGGER_ENTRY(CONFIG_SERVER_LOG_LEVEL, INFO, CONFIG_SERVER, "Config Server: Setting Log level %vl for module %vm") \
    LOGGER_ENTRY(CONFIG_SERVER_LOG_LEVEL_ALL, ALERT, SYSTEM, "Config Server: Setting Log level %vl for all modules") \
    LOGGER_ENTRY(CONFIG_SERVER_TERMINATE, ALERT, CONFIG_SERVER, "Config Server: Terminating...") \
    \
    LOGGER_ENTRY(SOCKET_SSL_INITIALIZE, INFO, SOCKET, "Socket initialize SSL") \
    LOGGER_ENTRY(SOCKET_SSL_INITIALIZE_ATTEMPT, DEBUG, SOCKET, "Socket initialize SSL attempt  %i") \
    LOGGER_ENTRY(SOCKET_SSL_CERT_LOAD_SUCCESS, INFO, SOCKET, "FD %i: Loaded SSL certificate") \
    LOGGER_ENTRY(SOCKET_SSL_CERT_LOAD_FAILED, ERROR, SOCKET, "FD %i: Unable to load SSL certificate, exiting") \
    LOGGER_ENTRY(SOCKET_SSL_CERT_LOAD_FAILED_FILE_NOT_FOUND, ERROR, SOCKET, "FD %i: Unable to load SSL certificate as file not found, exiting") \
    LOGGER_ENTRY(SOCKET_SSL_PRIKEY_LOAD_SUCCESS, INFO, SOCKET, "FD %i: Loaded Primary Key") \
    LOGGER_ENTRY(SOCKET_SSL_PRIKEY_LOAD_FAILED, ERROR, SOCKET, "FD %i: Unable to load Primary Key, exiting") \
    LOGGER_ENTRY(SOCKET_SSL_CLEANUP, INFO, SOCKET, "Socket cleanup SSL") \
    LOGGER_ENTRY(SOCKET_SSL_CLEANUP_ATTEMPT, DEBUG, SOCKET, "Socket cleanup SSL left %i") \
    LOGGER_ENTRY(SOCKET_SSL_ACCEPT_RETRY, DEBUG, SOCKET, "FD %i: SSL Socket accept retry SSL Accept") \
    LOGGER_ENTRY(SOCKET_SSL_ACCEPT_SUCCESS, VERBOSE, SOCKET, "FD %i: SSL Socket accept success") \
    LOGGER_ENTRY(SOCKET_SSL_ACCEPT_FAILED, ERROR, SOCKET, "FD %i: SSL Socket accept failed, with %vc") \
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
    LOGGER_ENTRY(EVENT_CREATE_FAILED, ERROR, EVENT_EXECUTOR, "FD %i: Event creation failed with error %ve") \
    LOGGER_ENTRY(EVENT_CREATE_SUCCESS, DEBUG, EVENT_EXECUTOR, "FD %i: Event creation succeeded") \
    LOGGER_ENTRY(EVENT_REMOVE_SUCCESS, DEBUG, EVENT_EXECUTOR, "FD %i: Event removal succeeded") \
    LOGGER_ENTRY(EVENT_REMOVE_FAILED, INFO, EVENT_EXECUTOR, "FD %i: Event removal failed with error %ve") \
    \
    LOGGER_ENTRY(EVENT_SERVER_RECEIVED_EVENT, DEBUG, EVENT_SERVER, "FD %i: Event server received event") \
    LOGGER_ENTRY(EVENT_SERVER_SSL_RECEIVED_EVENT, DEBUG, EVENT_SERVER, "FD %i: SSL Event server received event %vv") \
    LOGGER_ENTRY(EVENT_SERVER_ACCEPT_FAILED, ERROR, EVENT_SERVER, "FD %i: Event server failed to accept connection with error %ve") \
    LOGGER_ENTRY(EVENT_SERVER_SSL_ACCEPT_FAILED, ERROR, EVENT_SERVER, "SSL Event server failed to accept connection with error %ve") \
    LOGGER_ENTRY(EVENT_SERVER_PEER_CREATED, VERBOSE, EVENT_SERVER, "FD %i: Event server new peer %i created from remote %vN") \
    LOGGER_ENTRY(EVENT_SERVER_HELPER_WRITE_FAILED, WARNING, EVENT_SERVER, "Event server helper event write failed with error %ve") \
    LOGGER_ENTRY(EVENT_SERVER_HELPER_READ_FAILED, WARNING, EVENT_SERVER, "Event server helper event read failed with error %ve") \
    LOGGER_ENTRY(EVENT_SERVER_HELPER_UNKNOWN, WARNING, EVENT_SERVER, "Event server helper unknown message") \
    LOGGER_ENTRY(EVENT_SERVER_MOVED_ENTERED, WARNING, EVENT_SERVER, "FD %i: Entered event server after move, must not happen") \
    LOGGER_ENTRY(EVENT_SERVER_SSL_CLOSED_WRITE, INFO, EVENT_SERVER, "FD %i: SSL Event failed to write as socket is closed") \
    LOGGER_ENTRY(EVENT_SERVER_UNKNOWN_STATE, WARNING, EVENT_SERVER, "FD %i: Entered event server for unknown state %vs") \
    LOGGER_ENTRY(EVENT_SERVER_CONNECTION_CLOSED, INFO, IOT_EVENT_SERVER, "FD %i: Event Server connection closed") \
    \
    LOGGER_ENTRY(IOT_EVENT_SERVER_READ_FAILED, DEBUG, IOT_EVENT_SERVER, "IOT Event Server peer read failed with error %vE") \
    LOGGER_ENTRY(IOT_EVENT_SERVER_WRITE_FAILED, ERROR, IOT_EVENT_SERVER, "IOT Event Server peer write failed with error %vE") \
    \
    LOGGER_ENTRY(FILEWATCHER_EVENT_CREATE_FAILED, ERROR, IOT_HTTPSERVER, "FILEWATCHER event create failed") \
    LOGGER_ENTRY(FILEWATCHER_ADD_FOLDER_FAILED, WARNING, IOT_HTTPSERVER, "FILEWATCHER failed to add folder for watch with error %ve") \
    LOGGER_ENTRY(FILEWATCHER_EVENT_READ_FAILED, INFO, IOT_HTTPSERVER, "FILEWATCHER event read failed with error %ve") \
    LOGGER_ENTRY(HTTP_EVENT_SERVER_READ_FAILED, DEBUG, IOT_HTTPSERVER, "HTTP Event Server peer read failed with error %vE") \
    \
    LOGGER_ENTRY(HTTP2_EVENT_SERVER_READ_FAILED, DEBUG, IOT_HTTP2SERVER, "Socket %i: HTTP2 Event Server peer read failed with error %vE") \
    \
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

enum class logger_level : uint8_t {
#define LOGGER_LEVEL_ENTRY(x) x,
    LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY
};

constexpr const char *logger_level_string[] = {
#define LOGGER_LEVEL_ENTRY(x) {#x},
    LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY
};

inline logger_level to_logger_level(const std::string level_name) {
    static std::unordered_map<std::string, logger_level> logger_level_enum = {
#define LOGGER_LEVEL_ENTRY(x) {#x, logger_level::x},
        LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY
    };

    auto level_itr = logger_level_enum.find(level_name);
    if (level_itr == logger_level_enum.end()) {
        return static_cast<logger_level>(0xff);
    }

    return level_itr->second;
}

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

template <bool null_terminated = true>
constexpr size_t to_string(const logger_level &val, char *dest) {
    switch (val) {
    default: // This will avoid error, such condition will never reach
        assert(true);
#define LOGGER_LEVEL_ENTRY(x) \
    case logger_level::x: { \
        constexpr size_t desc_size = sizeof(#x) \
                - (null_terminated ? 0: 1); \
        constexpr const char desc[] = #x; \
        std::copy(desc, desc + desc_size, dest); \
        return desc_size; }
        LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY
    }
}

enum class module_t {
#define LOGGER_MODULE_ENTRY(x) x,
    LOGGER_MODULE_LIST
#undef LOGGER_MODULE_ENTRY
};

constexpr const char *module_t_string[] = {
#define LOGGER_MODULE_ENTRY(x) {#x},
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

template <bool null_terminated = true>
constexpr size_t to_string(const module_t &val, char *dest) {
    switch (val) {
    default: // This will avoid error, such condition will never reach
        assert(true);
#define LOGGER_MODULE_ENTRY(x) \
    case module_t::x: { \
        constexpr size_t desc_size = sizeof(#x) \
                - (null_terminated ? 0: 1); \
        constexpr const char desc[] = #x; \
        std::copy(desc, desc + desc_size, dest); \
        return desc_size; }
        LOGGER_MODULE_LIST
#undef LOGGER_MODULE_ENTRY
    }
}

} // namespace rohit