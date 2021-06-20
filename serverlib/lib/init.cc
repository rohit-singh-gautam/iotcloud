////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/init.hh>
#include <iot/core/log.hh>
#include <iot/states/event_distributor.hh>

namespace rohit {

void init_iot(const char *logfilename, const int thread_count) {
    init_log_thread(logfilename);
}


void destroy_iot() {
    destroy_log_thread();
}

} // namespace rohit