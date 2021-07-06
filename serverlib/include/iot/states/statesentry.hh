////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

namespace rohit {

#define STATE_DEFINITION_END

#define STATE_ENTRY_LIST \
    STATE_ENTRY(EVENT_DIST_NONE, "Not yet started") \
    STATE_ENTRY(EVENT_DIST_EPOLL_WAIT, "Waiting for Event") \
    STATE_DEFINITION_END


} // namespace rohit

