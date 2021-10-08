////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/net/serverevent.hh>
#include <iot/message.hh>

namespace rohit {

template <bool use_ssl>
class iotserverevent : public serverpeerevent<use_ssl> {
public:
    static constexpr bool movable = true;

private:
    using serverpeerevent<use_ssl>::peer_id;
    using serverpeerevent<use_ssl>::enter_loop;
    using serverpeerevent<use_ssl>::exit_loop;
    using serverpeerevent<use_ssl>::client_state;
    using serverpeerevent<use_ssl>::write_queue;

    using serverpeerevent_base::push_write;
    using serverpeerevent_base::pop_write;
    using serverpeerevent_base::get_write_buffer;
    using serverpeerevent_base::is_write_left;

public:
    using serverpeerevent<use_ssl>::serverpeerevent;
    
    using serverpeerevent<use_ssl>::write_all;

    void read_helper(thread_context &ctx);

    void execute(thread_context &ctx) override;

    using serverpeerevent<use_ssl>::close;
};

template <bool use_ssl>
void iotserverevent<use_ssl>::read_helper(thread_context &ctx) {
    size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    size_t read_buffer_length;

    auto err = peer_id.read(read_buffer, read_buffer_size, read_buffer_length);

    if (err == err_t::SOCKET_RETRY) {
        // No state change required
        return;
    }
    if (isFailure(err)) {
        ctx.log<log_t::IOT_EVENT_SERVER_READ_FAILED>(err);
        return;
    }

    if (read_buffer_length == 0) {
        // No data indication that wait
        return;
    }

    rohit::message_base_t *base = (rohit::message_base_t *)read_buffer;

    if constexpr (config::debug) {
        std::cout << "------Request Start---------\n" << *base << "\n------Request End---------\n";
    }

    size_t write_buffer_size = 0;
    uint8_t *write_buffer;

    if (*base != rohit::message_code_t::COMMAND) {
        write_buffer_size = sizeof(rohit::message_unknown_t);
        write_buffer = new uint8_t[write_buffer_size];
        rohit::message_unknown_t *punknownMessage = new (write_buffer) rohit::message_unknown_t();
    } else {
        write_buffer_size = sizeof(rohit::message_success_t);
        write_buffer = new uint8_t[write_buffer_size];
        rohit::message_success_t *psuccessMessage = new (write_buffer) rohit::message_success_t();
    }

    if constexpr (config::debug) {
        message_base_t *write_base = (message_base_t *)write_buffer;
        std::cout << "------Response Start---------\n" << *write_base << "\n------Response End---------\n";
    }

    push_write(write_buffer, write_buffer_size);

    write_all(ctx);

    // Tail recurssion
    read_helper(ctx);
}

template <bool use_ssl>
void iotserverevent<use_ssl>::execute(thread_context &ctx) {
    switch (client_state) {
        case state_t::SOCKET_PEER_ACCEPT: {
            auto err = peer_id.accept();
            if (err == err_t::SUCCESS) {
                client_state = state_t::SOCKET_PEER_EVENT;
            } else {
                close(ctx);
            }
            break;
        }
        case state_t::SOCKET_PEER_CLOSE: {
            close(ctx);
            break;
        }
        case state_t::SOCKET_PEER_EVENT:
        case state_t::SOCKET_PEER_READ: {
            read_helper(ctx);
            break;
        }
        case state_t::SOCKET_PEER_WRITE: {
            write_all(ctx);
            read_helper(ctx);
            break;
        }

    }
}

} // namespace rohit