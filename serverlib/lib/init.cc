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

#include <iot/init.hh>
#include <iot/core/log.hh>
#include <iot/states/event_distributor.hh>
#include <iot/net/socket.hh>

namespace rohit {

extern void init_openssl();
extern void cleanup_openssl();

void init_iot(const char *logfilename, const int thread_count) {
    init_log_thread(logfilename);
    if constexpr (config::enable_ssl) socket_ssl_t::init_openssl();
}


void destroy_iot() {
    destroy_log_thread();
    if constexpr (config::enable_ssl) socket_ssl_t::cleanup_openssl();
}

} // namespace rohit