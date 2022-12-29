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

constexpr uint8_t web_server_name[] { WEB_SERVER_NAME };
constexpr const char ipc_path[] { "/tmp/iotcloud/pipe/config" };

} // namespace config
} // namespace rohit