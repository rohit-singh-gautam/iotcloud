////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <http11driver.hh>
#include <iothttphelper.hh>
#include <iot/net/serverevent.hh>
#include <iotfilemapping.hh>
#include <iot/message.hh>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>

namespace rohit {

struct iothttpevent_config {
    const ipv6_port_t port;
    const std::string web_folder;
    const std::string default_file;
    inline iothttpevent_config(const ipv6_port_t port, const std::string &web_folder, const std::string &default_file)
        : port(port), web_folder(web_folder), default_file(default_file) { }

    inline iothttpevent_config(const uint16_t port, const std::string &web_folder, const std::string &default_file)
        : port(port), web_folder(web_folder), default_file(default_file) { }

    inline iothttpevent_config(const int port, const std::string &web_folder, const std::string &default_file)
                : port(port), web_folder(web_folder), default_file(default_file) {
        assert(port > 65535);
    }

    bool operator==(const iothttpevent_config &rhs) {
        return port == port && web_folder == web_folder && default_file == default_file;
    }
};

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
            constexpr size_t read_buffer_size = 1024*1024*5; // 5MB Stack
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
            std::cout << "------Request Start---------\n" << std::string(read_buffer, read_buffer_length)  << "\n------Request End---------\n";

            http11driver driver;
            driver.parse(request_string);
            std::cout << "------Driver Start---------\n" << driver << "\n------Driver End---------\n";

            // Date is used by all hence it is created here
            std::time_t now_time = std::time(0);   // get time now
            std::tm* now_tm = std::gmtime(&now_time);
            char date_str[config::max_date_string_size];
            size_t date_str_size = strftime(date_str, config::max_date_string_size, "%a, %d %b %Y %H:%M:%S %Z", now_tm) + 1;

            auto local_address = peer_id.get_local_ipv6_addr();
            size_t write_size = 0;
            if (driver.header.method == rohit::http_header_request::METHOD::GET) {
                auto port = local_address.port;
                rohit::http::file_map_param map_param(port, driver.header.path);

                auto result = rohit::http::webfilemap.cache.find(map_param);
                if (result == rohit::http::webfilemap.cache.end()) {
                    auto last_write_buffer = http_add_404_Not_Found(read_buffer, local_address, date_str, date_str_size);
                    write_size = (size_t)(last_write_buffer - read_buffer);
                } else {
                    auto file_details = result->second;

                    if (driver.header.match_etag(file_details->etags, file_details->etags_size)) {
                        const http_header_line header_line[] = {
                            {http_header::FIELD::Date, date_str, date_str_size},
                            {http_header::FIELD::Server, config::web_server_name},
                            {http_header::FIELD::ETag, file_details->etags, file_details->etags_size},
                        };
                        char *last_write_buffer = copy_http_header_response(
                            read_buffer,
                            http_header::VERSION::VER_1_1,
                            304_rc,
                            header_line
                        );

                        *last_write_buffer++ = '\n';

                        write_size = (size_t)(last_write_buffer - read_buffer);
                        std::cout << "------Response Start---------\n" << std::string(read_buffer, write_size) << "\n------Response End---------\n";
                    } else {
                        const http_header_line header_line[] = {
                            {http_header::FIELD::Date, date_str, date_str_size},
                            {http_header::FIELD::Server, config::web_server_name},
                            {http_header::FIELD::Cache_Control, "private, max-age=2592000"},
                            {http_header::FIELD::ETag, file_details->etags, rohit::http::file_info::etags_size},
                            {http_header::FIELD::Content_Type, file_details->type, file_details->type_size},
                        };

                        char *last_write_buffer = copy_http_response(
                            read_buffer,
                            http_header::VERSION::VER_1_1,
                            200_rc,
                            header_line,
                            file_details->text,
                            file_details->text_size
                        );

                        write_size = (size_t)(last_write_buffer - read_buffer);
                    }
                }
            }
            else {
                auto last_write_buffer = http_add_404_Not_Found(read_buffer, local_address, date_str, date_str_size);
                write_size = (size_t)(last_write_buffer - read_buffer);
            }

            uint8_t *write_buffer = new uint8_t[write_size];
            std::copy(read_buffer, read_buffer + write_size, write_buffer);

            size_t written_length;
            err = peer_id.write(write_buffer, write_size, written_length);
            if (err == err_t::SOCKET_RETRY) {
                write_queue.push({write_buffer, written_length, write_size});
                err = err_t::SUCCESS;
                client_state = state_t::SOCKET_PEER_WRITE;
            } else {
                client_state = state_t::SOCKET_PEER_EVENT;
            }

            if (isFailure(err)) {
                ctx.log<log_t::IOT_EVENT_SERVER_WRITE_FAILED>(err);
            }
            break;
        }

        case state_t::SOCKET_PEER_WRITE: {
            err_t err = err_t::SUCCESS;
            client_state = state_t::SOCKET_PEER_EVENT;
            while (!write_queue.empty()) {
                auto &write_buffer = write_queue.front();

                size_t written_length;
                size_t write_size = write_buffer.size - write_buffer.written;
                err = peer_id.write(write_buffer.buffer + write_buffer.written, write_size, written_length);
                if (err == err_t::SUCCESS) {
                    assert(written_length == write_size);
                    write_queue.pop();
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
                    write_queue.pop();
                    delete[] write_buffer.buffer;
                }
            }
            break;
        }

    }

    unlock();
}

} // namespace rohit