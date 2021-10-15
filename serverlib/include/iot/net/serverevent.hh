////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/pthread_helper.hh>
#include <iot/states/event_distributor.hh>
#include <iot/states/statesentry.hh>
#include <iot/states/states.hh>
#include <iot/net/socket.hh>
#include <atomic>

namespace rohit {


template <typename peerevent, bool use_ssl, bool use_lock = use_ssl>
class serverevent : public event_executor, public pthread_lock_c<use_lock> {
private:
    server_socket_variant_t<use_ssl>::type socket_id;
    const int port;
    const int maxconnection;

public:
    serverevent(const int port,
                const int maxconnection = 10000);

    serverevent(const int port,
                const char *const cert_file,
                const char *const prikey_file,
                const int maxconnection = 10000);

    inline void init(event_distributor &evtdist) {
        socket_id.set_non_blocking();
        evtdist.add(socket_id, EPOLLIN, this);
    }


    void execute(thread_context &ctx) override {
        ctx.log<log_t::EVENT_SERVER_RECEIVED_EVENT>((int)socket_id);
        try {
            while(true) {
                auto peer_id = socket_id.accept();
                if (peer_id.is_null()) break;
                const auto non_blocking = peer_id.set_non_blocking();
                if (!non_blocking) {
                    ctx.log<log_t::SOCKET_SET_NONBLOCKING_FAILED>((int)peer_id);
                }
                peerevent *p_peerevent = new peerevent(peer_id);
                p_peerevent->execute_protector(ctx);
                if constexpr (peerevent::movable) {
                    if (p_peerevent->get_client_state() != state_t::SERVEREVENT_MOVED) {
                        ctx.add_event(peer_id, EPOLLIN | EPOLLOUT, p_peerevent);
                    }
                } else {
                    ctx.add_event(peer_id, EPOLLIN | EPOLLOUT, p_peerevent);
                }
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

    void close(thread_context &ctx) override {
        socket_id.close();
        ctx.delayed_free(this);
    }
};

template <typename peerevent, bool use_ssl, bool use_lock>
inline serverevent<peerevent, use_ssl, use_lock>::serverevent(
    const int port,
    const int maxconnection)
        :   socket_id(port),
            port(port),
            maxconnection(maxconnection) {
    static_assert(!use_ssl, "Provide cert_file and prikey_file parameters");
}

template <typename peerevent, bool use_ssl, bool use_lock>
inline serverevent<peerevent, use_ssl, use_lock>::serverevent(
        const int port,
        const char *const cert_file,
        const char *const prikey_file,
        const int maxconnection)
            :   socket_id(port, cert_file, prikey_file),
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

template <bool use_ssl>
class serverpeerevent : public event_executor, public serverpeerevent_base {
protected:
    socket_variant_t<use_ssl>::type peer_id;
    state_t client_state;

public:
    inline serverpeerevent(socket_variant_t<use_ssl>::type &peer_id)
              : peer_id(peer_id),
                client_state(use_ssl ? state_t::SOCKET_PEER_ACCEPT : state_t::SOCKET_PEER_READ) { }

    inline serverpeerevent(serverpeerevent &&peerevent)
        :   serverpeerevent_base(std::move(peerevent)),
            peer_id(std::move(peerevent.peer_id)),
            client_state(peerevent.client_state) { 
        peerevent.client_state = state_t::SERVEREVENT_MOVED;
    }

    constexpr const state_t get_client_state() const { return client_state; }

    void write_all(thread_context &ctx);

    void close(thread_context &ctx) override;

}; // class serverpeerevent

template <bool use_ssl>
void serverpeerevent<use_ssl>::close(thread_context &ctx) {
    int last_peer_id = peer_id;
    if (last_peer_id) {
        auto ret = peer_id.close();
        if (ret != err_t::SOCKET_RETRY) {
            ctx.log<log_t::IOT_EVENT_SERVER_CONNECTION_CLOSED>((int)last_peer_id);
            client_state = state_t::SOCKET_PEER_CLOSED;
            ctx.delayed_free(this);
        } else {
            client_state = state_t::SOCKET_PEER_CLOSE;
        }
    }
}

template <bool use_ssl>
void serverpeerevent<use_ssl>::write_all(thread_context &ctx) {
    err_t err = err_t::SUCCESS;
    client_state = state_t::SOCKET_PEER_EVENT;
    while (is_write_left()) {
        auto &write_buffer = get_write_buffer();

        size_t written_length;
        size_t write_size = write_buffer.size - write_buffer.written;
        err = peer_id.write(write_buffer.buffer + write_buffer.written, write_size, written_length);
        if (err == err_t::SUCCESS) {
            assert(written_length == write_size);
            pop_write();
            delete[] write_buffer.buffer;
        } else if (err == err_t::SOCKET_RETRY) {
            assert(written_length <= write_size);
            write_buffer.written += written_length;
            err = err_t::SUCCESS;
            client_state = state_t::SOCKET_PEER_WRITE;
            break;
        } else if (isFailure(err)) {
            ctx.log<log_t::IOT_EVENT_SERVER_WRITE_FAILED>(err);
            // Removing from write queue
            pop_write();
            delete[] write_buffer.buffer;
        }
    }
}

} // namespace rohit