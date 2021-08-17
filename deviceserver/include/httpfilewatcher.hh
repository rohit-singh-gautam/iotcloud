////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/watcher/filewatcherevent.hh>
#include <iotfilemapping.hh>

namespace rohit::http {

class httpfilewatcher : public filewatcherevent<httpfilewatcher> {
public:
    using filewatcherevent::filewatcherevent;

    inline void receive_event(const std::string &watchfolder, uint32_t eventmask, const char *const name, uint32_t name_len) {
        // Do nothing here, TODO: Optimization for updating only what is changed
    }

    inline void receive_event_finalize(const std::string &watchfolder) {
        webfilemap.flush_cache(watchfolder);
        webfilemap.update_folder(watchfolder);
    }

    using filewatcherevent::add_folder;
};

} // namespace rohit::http