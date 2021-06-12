////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/states/event_distributor.hh>
#include "socket.hh"

namespace rohit {

template <typename peerevent>
class serverevent : public event_executor {
private:
    event_distributor &evtdist;
    server_socket_t socket_id;
    const int port;
    const int maxconnection;
    const int backlog;

public:
    inline serverevent(event_distributor &evtdist, const int port, const int maxconnection = 10000, const int backlog = 5)
            :   evtdist(evtdist),
                socket_id(port, backlog, false),
                port(port),
                maxconnection(maxconnection),
                backlog(backlog) {
        evtdist.add(socket_id, event_hook_t::IN, *this);
    }

    void execute(thread_context &ctx) override {
        try {
            socket_t peer_id = socket_id.accept();
            peerevent *p_peerevent = mem.alloc<peerevent>(evtdist, peer_id); 
        } catch (const exception_t e) {
            if (e == err_t::ACCEPT_FAILURE) {
                ctx.log<log_t::EVENT_SERVER_ACCEPT_FAILED>(errno);
            }
        }
    }
};

class serverpeerevent : public event_executor {
protected:
    socket_t peer_id;

public:
    inline serverpeerevent(event_distributor &evtdist, socket_t peer_id) : peer_id(peer_id) {
        evtdist.add(peer_id, event_hook_t::IN, *this);
    }

};

} // namespace rohit