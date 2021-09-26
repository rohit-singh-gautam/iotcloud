////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

namespace rohit {

#define STATE_DEFINITION_END

#define STATE_ENTRY_LIST \
    STATE_ENTRY(STATE_UNKNOWN, "Unknown state") \
    \
    STATE_ENTRY(EVENT_DIST_NONE, "Not yet started") \
    STATE_ENTRY(EVENT_DIST_EPOLL_WAIT, "Epoll waiting for Event") \
    STATE_ENTRY(EVENT_DIST_EPOLL_TERMINATE, "Epoll Terminated") \
    STATE_ENTRY(EVENT_DIST_EPOLL_ERROR, "Epoll error") \
    STATE_ENTRY(EVENT_DIST_EPOLL_PROCESSING, "Epoll processing") \
    STATE_ENTRY(EVENT_DIST_EPOLL_CLOSE, "Epoll close") \
    STATE_ENTRY(EVENT_DIST_EPOLL_EXECUTE, "Epoll execute") \
    \
    STATE_ENTRY(SOCKET_PEER_ACCEPT, "Accept not completed, happens with SSL") \
    STATE_ENTRY(SOCKET_PEER_EVENT, "Next operation is based on event type") \
    STATE_ENTRY(SOCKET_PEER_READ, "Next operation is READ") \
    STATE_ENTRY(SOCKET_PEER_WRITE, "Next operation is WRITE") \
    STATE_ENTRY(SOCKET_PEER_CLOSE, "Next operation is CLOSE") \
    STATE_ENTRY(SOCKET_PEER_CLOSED, "Socket is close no further operation must be performed on this") \
    STATE_ENTRY(HTTP2_SETTINGS_WRITE, "HTTP2 write settings as this is first packet") \
    STATE_ENTRY(HTTP2_NEXT_MAGIC, "HTTP2 next frame is a magic frame") \
    STATE_ENTRY(SERVEREVENT_MOVED, "SERVEREVENT is moved to another object") \
    \
    STATE_DEFINITION_END


} // namespace rohit

