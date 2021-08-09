////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/version.h>
#include <stdint.h>

namespace rohit {
namespace config {

constexpr bool debug = true;
constexpr bool enable_ssl = true;
constexpr bool log_with_check = false;
constexpr int64_t log_thread_wait_in_millis = 50;
constexpr int64_t event_dist_loop_wait_in_millis = 10;
constexpr int64_t event_dist_deadlock_in_nanos = 10000LL * 1000000LL;
constexpr uint64_t event_cleanup_time_in_ns = 2ULL * 1000ULL * 1000000ULL; // 2 second
constexpr uint64_t attempt_to_write = 20;
constexpr int64_t attempt_to_write_wait_in_ms = 50;
constexpr int socket_read_buffer_size = 25 * 1024 * 1024; // Read buffer setting it to 25MB
constexpr int socket_write_buffer_size = 25 * 1024 * 1024; // weite buffer setting it to 25MB
constexpr int socket_backlog = 5;
constexpr uint64_t max_date_string_size = 92;
constexpr int64_t filewatcher_wait_in_ns = 1ULL * 1000ULL * 1000000ULL;

#define macrostr_helper(x) #x
#define macrostr(x) macrostr_helper(x)
#define WEB_SERVER_NAME "Rohit Web " macrostr(IOT_VERSION_MAJOR) "." macrostr(IOT_VERSION_MINOR)

constexpr char web_server_name[] = WEB_SERVER_NAME;

} // namespace config
} // namespace rohit