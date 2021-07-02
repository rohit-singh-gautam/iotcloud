////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/net/serverevent.hh>

namespace rohit {

class iotserverevent : public serverpeerevent {
public:
    using serverpeerevent::serverpeerevent;
    
    void execute(thread_context &ctx, const uint32_t event) override;

    void close(thread_context &ctx);

};

} // namespace rohit