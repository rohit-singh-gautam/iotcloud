////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <http11driver.hh>
#include <iot/net/serverevent.hh>
#include <iot/message.hh>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>

namespace rohit {

template <bool use_ssl>
class iothttpevent : public serverpeerevent<use_ssl> {
private:
    using serverpeerevent<use_ssl>::peer_id;
    using serverpeerevent<use_ssl>::lock;
    using serverpeerevent<use_ssl>::unlock;
    using serverpeerevent<use_ssl>::client_state;
    using serverpeerevent<use_ssl>::write_queue;

public:
    using serverpeerevent<use_ssl>::serverpeerevent;
    
    void execute(thread_context &ctx, const uint32_t event) override;

    void close(thread_context &ctx);
};

template <bool use_ssl>
void iothttpevent<use_ssl>::close(thread_context &ctx) {
    int last_peer_id = peer_id;
    if (last_peer_id) {
        auto ret = peer_id.close();
        if (ret != err_t::SOCKET_RETRY) {
            ctx.log<log_t::IOT_EVENT_SERVER_CONNECTION_CLOSED>((int)last_peer_id);
            client_state = state_t::SOCKET_PEER_CLOSED;
        } else {
            client_state = state_t::SOCKET_PEER_CLOSE;
        }
    }
}

template <bool use_ssl>
void iothttpevent<use_ssl>::execute(thread_context &ctx, const uint32_t event) {
    lock();
    if ((event & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) != 0) {
        // TODO: Database has to be update with information that connection is closed
        close(ctx);
        unlock();
        return;
    }

    // if state is SOCKET_PEER_EVENT change state based on event
    if (client_state == state_t::SOCKET_PEER_EVENT) {
        ctx.log<log_t::IOT_EVENT_SERVER_COMMAND_RECEIVED>(peer_id.get_peer_ipv6_addr());
        if (event & EPOLLIN) client_state = state_t::SOCKET_PEER_READ;
        else if (event & EPOLLOUT) client_state = state_t::SOCKET_PEER_WRITE;
    }

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

        case state_t::SOCKET_PEER_READ: {
            constexpr size_t read_buffer_size = 1024;
            char read_buffer[read_buffer_size];
            size_t read_buffer_length;

            auto err = peer_id.read(read_buffer, read_buffer_size, read_buffer_length);

            if (err == err_t::SOCKET_RETRY) {
                // No state change required
                break;
            }
            if (isFailure(err)) {
                ctx.log<log_t::IOT_EVENT_SERVER_READ_FAILED>(err);
                client_state = state_t::SOCKET_PEER_EVENT;
                break;
            }

            if (read_buffer_length == 0) {
                // No data indication that wait
                client_state = state_t::SOCKET_PEER_EVENT;
                break;
            }

            std::string request_string((char *)read_buffer, read_buffer_length);
            std::cout << "------Request Start---------\n" << request_string << "\n------Request End---------\n";

            http11driver driver;
            driver.parse(request_string);
            std::cout << "------Driver Start---------\n" << driver << "\n------Driver End---------\n";

            const http_header_line header_line[] = {
                {http_header::FIELD::Server, config::server_name},
                {http_header::FIELD::Content_Type, "application/json"},
            };

            char *last_write_buffer = copy_http_response(
                read_buffer,
                http_header::VERSION::VER_1_1,
                200_rc,
                header_line,
                "{result:""success""}\n"
            );

            auto write_size = (size_t)(last_write_buffer - read_buffer);

            std::cout << "------Response Start---------\n" << read_buffer << "\n------Response End---------\n";


            size_t written_length;
            err = peer_id.write(read_buffer, write_size, written_length);
            if (err == err_t::SOCKET_RETRY) {
                size_t write_buffer_size = write_size;
                uint8_t *write_buffer = new uint8_t[write_buffer_size];
                std::copy(read_buffer, read_buffer + write_buffer_size, write_buffer);
                write_queue.push({write_buffer, write_buffer_size});
                break;
            }

            if (isFailure(err)) {
                ctx.log<log_t::IOT_EVENT_SERVER_WRITE_FAILED>(errno);
            }

            client_state = state_t::SOCKET_PEER_EVENT;
            break;
        }

        case state_t::SOCKET_PEER_WRITE: {
            err_t err = err_t::SUCCESS;
            while (!write_queue.empty()) {

                auto write_buffer = write_queue.front();

                size_t written_length;
                err = peer_id.write(write_buffer.buffer, write_buffer.size, written_length);
                if (err == err_t::SOCKET_RETRY) {
                    break;
                }
                write_queue.pop();
                delete[] write_buffer.buffer;

                if (isFailure(err)) {
                    ctx.log<log_t::IOT_EVENT_SERVER_WRITE_FAILED>(errno);
                }
            }

            if (err != err_t::SOCKET_RETRY) client_state = state_t::SOCKET_PEER_EVENT;
            break;
        }

    }

    unlock();
}

} // namespace rohit