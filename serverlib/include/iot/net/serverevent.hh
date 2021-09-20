////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/pthread_helper.hh>
#include <iot/states/event_distributor.hh>
#include <iot/states/statesentry.hh>
#include <iot/states/states.hh>
#include "socket.hh"

namespace rohit {


template <typename peerevent, bool use_ssl, bool use_lock = use_ssl>
class serverevent : public event_executor, public pthread_lock_c<use_lock> {
private:
    event_distributor &evtdist;
    server_socket_variant_t<use_ssl>::type socket_id;
    const int port;
    const int maxconnection;

public:
    serverevent(event_distributor &evtdist,
                const int port,
                const int maxconnection = 10000);

    serverevent(event_distributor &evtdist,
                const int port,
                const char *const cert_file,
                const char *const prikey_file,
                const int maxconnection = 10000);

    inline void init() {
        socket_id.set_non_blocking();
        evtdist.add(socket_id, EPOLLIN, *this);
    }


    void execute(thread_context &ctx, const uint32_t event) override {
        ctx.log<log_t::EVENT_SERVER_RECEIVED_EVENT>((int)socket_id, event);
        if ((event & (EPOLLHUP | EPOLLRDHUP)) != 0) {
            ctx.log<log_t::EVENT_SERVER_RECEIVED_CLOSED>();
            return;
        }
        try {
            while(true) {
                auto peer_id = socket_id.accept();
                if (peer_id.is_null()) break;
                peerevent *p_peerevent = new peerevent(std::move(peer_id));
                peer_id.set_non_blocking();
                evtdist.add(peer_id, EPOLLIN | EPOLLOUT, *p_peerevent);
                ctx.log<log_t::EVENT_SERVER_PEER_CREATED>(peer_id.get_peer_ipv6_addr());
            }
        } catch (const exception_t e) {
            if (e == err_t::ACCEPT_FAILURE) {
                ctx.log<log_t::EVENT_SERVER_ACCEPT_FAILED>(errno);
            }
        }
    }

    void close() {
        socket_id.close();
    }
};

template <typename peerevent, bool use_ssl, bool use_lock>
inline serverevent<peerevent, use_ssl, use_lock>::serverevent(
    event_distributor &evtdist,
    const int port,
    const int maxconnection)
        :   evtdist(evtdist),
            socket_id(port),
            port(port),
            maxconnection(maxconnection) {
    static_assert(!use_ssl, "Provide cert_file and prikey_file parameters");
}

template <typename peerevent, bool use_ssl, bool use_lock>
inline serverevent<peerevent, use_ssl, use_lock>::serverevent(
        event_distributor &evtdist,
        const int port,
        const char *const cert_file,
        const char *const prikey_file,
        const int maxconnection)
            :   evtdist(evtdist),
                socket_id(port, cert_file, prikey_file),
                port(port),
                maxconnection(maxconnection) {
    static_assert(use_ssl, "cert_file and prikey_file parameters require only for SSL");
}

class serverpeerevent_base {
protected:
    struct write_entry {
        uint8_t *buffer;
        size_t written;
        size_t size;
    };

    std::queue<write_entry> write_queue;

public:
    inline serverpeerevent_base() : write_queue() { }
    inline serverpeerevent_base(serverpeerevent_base &&old) : write_queue(std::move(old.write_queue)) { }

    inline void push_write(uint8_t *buffer, size_t size) {
        write_queue.push({buffer, 0, size});
    }

    inline void pop_write() { write_queue.pop(); }

    inline write_entry &get_write_buffer() { return write_queue.front(); }

    inline bool is_write_left() { return !write_queue.empty(); }

};

template <bool use_ssl, bool use_lock = use_ssl>
class serverpeerevent : public serverpeerevent_base, public event_executor, public pthread_lock_c<use_lock> {
protected:
    socket_variant_t<use_ssl>::type peer_id;
    state_t client_state;

public:
    inline serverpeerevent(socket_variant_t<use_ssl>::type &&peer_id)
              : peer_id(std::move(peer_id)),
                client_state(use_ssl ? state_t::SOCKET_PEER_ACCEPT : state_t::SOCKET_PEER_READ) { }

    inline serverpeerevent(serverpeerevent &&peerevent)
        :   serverpeerevent_base(std::move(peerevent)),
            peer_id(std::move(peerevent.peer_id)),
            client_state(peerevent.client_state) { }

};

} // namespace rohit