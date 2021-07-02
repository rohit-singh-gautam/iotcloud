////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

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