////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once
#include <stdint.h>

namespace rohit {
namespace config {

constexpr bool debug = true;
constexpr bool enable_ssl = true;
constexpr bool log_with_check = false;
constexpr int64_t log_thread_wait_in_millis = 50;
constexpr int64_t event_dist_loop_wait_in_millis = 10;
constexpr int64_t event_dist_deadlock_in_nanos = 10000LL * 1000000LL;
constexpr uint64_t event_cleanup_time_in_ns = 30ULL * 1000ULL * 1000000ULL; // 60 second
constexpr uint64_t attempt_to_write = 10;
constexpr int socket_backlog = 5;

};
};