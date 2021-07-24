////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/states/event_distributor.hh>
#include <iot/core/memory.hh>
#include "socket.hh"

namespace rohit {

template <typename peerevent>
class serverevent : public event_executor {
private:
    event_distributor &evtdist;
    server_socket_t socket_id;
    const int port;
    const int maxconnection;

public:
    inline serverevent(event_distributor &evtdist, const int port, const int maxconnection = 10000)
            :   evtdist(evtdist),
                socket_id(port),
                port(port),
                maxconnection(maxconnection) {
    }

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
                socket_t peer_id = socket_id.accept();
                if (peer_id.is_null()) break;
                peerevent *p_peerevent = new peerevent(peer_id);
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

template <typename peerevent>
class serverevent_ssl : public event_executor {
private:
    pthread_mutex_t _lock;
    event_distributor &evtdist;
    server_socket_ssl_t socket_id;
    const int port;
    const int maxconnection;

public:
    inline serverevent_ssl(
        event_distributor &evtdist,
        const int port,
        const char *const cert_file,
        const char *const prikey_file,
        const int maxconnection = 10000)
            :   evtdist(evtdist),
                socket_id(port, cert_file, prikey_file),
                port(port),
                maxconnection(maxconnection) {
        pthread_mutex_init(&_lock, nullptr);
    }

    ~serverevent_ssl() {
        pthread_mutex_destroy(&_lock);
    }
    
    inline void init() {
        socket_id.set_non_blocking();
        evtdist.add(socket_id, EPOLLIN, *this);
    }

    inline void lock() {
        pthread_mutex_lock(&_lock);
    }

    inline void unlock() {
        pthread_mutex_unlock(&_lock);
    }

    void execute(thread_context &ctx, const uint32_t event) override {
        lock();
        ctx.log<log_t::EVENT_SERVER_SSL_RECEIVED_EVENT>((int)socket_id, event);
        if ((event & EPOLLHUP) == EPOLLHUP) return;
        try {
            while(true) {
                socket_ssl_t peer_id = socket_id.accept();
                if (peer_id == 0) break;
                peerevent *p_peerevent = new peerevent(peer_id);
                peer_id.set_non_blocking();
                evtdist.add(peer_id, EPOLLIN | EPOLLOUT, *p_peerevent);
                ctx.log<log_t::EVENT_SERVER_SSL_PEER_CREATED>(peer_id.get_peer_ipv6_addr());
            }
        } catch (const exception_t e) {
            if (e == err_t::ACCEPT_FAILURE) {
                ctx.log<log_t::EVENT_SERVER_SSL_ACCEPT_FAILED>(errno);
            }
        }
        unlock();
    }

    void close() {
        socket_id.close();
    }
};

class serverpeerevent : public event_executor {
protected:
    socket_t peer_id;

public:
    inline serverpeerevent(socket_t peer_id) 
            : peer_id(peer_id) {}

};

class serverpeerevent_ssl : public event_executor {
protected:
    socket_ssl_t peer_id;
    pthread_mutex_t _lock;

public:
    inline serverpeerevent_ssl(socket_ssl_t peer_id)
            : peer_id(peer_id) {
        pthread_mutex_init(&_lock, nullptr);
    }

    ~serverpeerevent_ssl() {
        pthread_mutex_destroy(&_lock);
    }

    inline void lock() {
        pthread_mutex_lock(&_lock);
    }

    inline void unlock() {
        pthread_mutex_unlock(&_lock);
    }

};

} // namespace rohit