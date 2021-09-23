////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <http11driver.hh>
#include <iothttphelper.hh>
#include <iot/net/serverevent.hh>
#include <http2.hh>
#include <iotfilemapping.hh>
#include <iot/message.hh>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>

namespace rohit {

class iotsslevent : public serverpeerevent<true> {
private:
    using serverpeerevent<true>::peer_id;
    using serverpeerevent<true>::lock;
    using serverpeerevent<true>::unlock;
    using serverpeerevent<true>::client_state;
    using serverpeerevent<true>::write_queue;

public:
    using serverpeerevent<true>::serverpeerevent;

    void execute(thread_context &ctx, const uint32_t event) override;

    void close(thread_context &ctx);
};

template <bool use_ssl>
class iothttpevent : public serverpeerevent<use_ssl> {
protected:
    using serverpeerevent<use_ssl>::peer_id;
    using serverpeerevent<use_ssl>::lock;
    using serverpeerevent<use_ssl>::unlock;
    using serverpeerevent<use_ssl>::client_state;
    using serverpeerevent<use_ssl>::write_queue;

    using serverpeerevent_base::push_write;
    using serverpeerevent_base::pop_write;
    using serverpeerevent_base::get_write_buffer;
    using serverpeerevent_base::is_write_left;

public:
    inline iothttpevent(socket_variant_t<use_ssl>::type &&peer_id) : serverpeerevent<use_ssl>(std::move(peer_id)) { }
    inline iothttpevent(iotsslevent &&sslevent) : iothttpevent<use_ssl>(std::move(sslevent)) {
        static_assert(use_ssl == false, "Only SSL event allowed, this function is for ALPN");
    }

    void write_all(thread_context &ctx);

    void execute(thread_context &ctx, const uint32_t event) override;

    void close(thread_context &ctx);
};

template <bool use_ssl>
class iothttp2event : public iothttpevent<use_ssl> {
protected:
    using serverpeerevent<use_ssl>::peer_id;
    using serverpeerevent<use_ssl>::lock;
    using serverpeerevent<use_ssl>::unlock;
    using serverpeerevent<use_ssl>::client_state;
    using serverpeerevent<use_ssl>::write_queue;

    using serverpeerevent_base::push_write;
    using serverpeerevent_base::pop_write;
    using serverpeerevent_base::get_write_buffer;
    using serverpeerevent_base::is_write_left;

    rohit::http::v2::dynamic_table_t dynamic_table;
    rohit::http::v2::settings_store peer_settings;

    friend class iothttpevent<use_ssl>;

public:
    inline iothttp2event(socket_variant_t<use_ssl>::type &&peer_id) : iothttpevent<use_ssl>(std::move(peer_id)) { }
    inline iothttp2event(iotsslevent &&sslevent) : iothttpevent<use_ssl>(std::move(sslevent)) { }
    inline iothttp2event(iothttpevent<use_ssl> &&http11event) : iothttpevent<use_ssl>(std::move(http11event)) { }

    using iothttpevent<use_ssl>::write_all;

    void process_request(
                thread_context &ctx,
                rohit::http::v2::request &request,
                uint8_t *pwrite_end);

    void process_read_buffer(thread_context &ctx, uint8_t *read_buffer, const size_t read_buffer_size);
    void upgrade(thread_context &ctx, const http_header_request &header, uint8_t *read_buffer);

    void execute(thread_context &ctx, const uint32_t event) override;

    using iothttpevent<use_ssl>::close;
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
void iothttpevent<use_ssl>::write_all(thread_context &ctx) {
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
            // TODO: This must be moved out of this function
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
            constexpr size_t read_buffer_size = 1024*1024*10; // 10MB Stack
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
            auto parserret = driver.parse(request_string);
            std::cout << "------Driver Start---------\n" << driver << "\n------Driver End---------\n";

            // Date is used by all hence it is created here
            std::time_t now_time = std::time(0);   // get time now
            std::tm* now_tm = std::gmtime(&now_time);
            char date_str[config::max_date_string_size];
            size_t date_str_size = strftime(date_str, config::max_date_string_size, "%a, %d %b %Y %H:%M:%S %Z", now_tm) + 1;

            auto local_address = peer_id.get_local_ipv6_addr();
            size_t write_size = 0;
            if (parserret != err_t::SUCCESS) {
                auto last_write_buffer = http_add_400_Bad_Request(read_buffer, local_address, date_str, date_str_size);
                write_size = (size_t)(last_write_buffer - read_buffer);
            } if (driver.header.method == rohit::http_header_request::METHOD::PRI) {
                if constexpr(use_ssl) {
                    // Reply in HTTP 1.1 only
                    auto last_write_buffer = http_add_400_Bad_Request(read_buffer, local_address, date_str, date_str_size);
                    write_size = (size_t)(last_write_buffer - read_buffer);
                } else {
                    if (driver.header.version == http_header::VERSION::VER_2) {
                        // Move to HTTP 2
                        ctx.remove_event(peer_id);

                        // below will move current structure to std::move
                        iothttp2event<use_ssl> *http2executor = new iothttp2event<use_ssl>(std::move(*this));

                        // Now we can unlock as no new call will come for this connection
                        // Also, this will free up other thread if hold up in lock
                        // As this thread is moved it is quivalent to close
                        // Even event is removed so no new event will come in
                        // We are avoiding one lock in this way
                        unlock();

                        const std::string &settings = driver.header.fields[http_header::FIELD::HTTP2_Settings];

                        // Execute all read and write
                        http2executor->process_read_buffer(
                                    ctx,
                                    (uint8_t *)read_buffer + rohit::http::v2::connection_preface_size,
                                    read_buffer_length - rohit::http::v2::connection_preface_size);

                        // Add http2executor to epoll
                        ctx.add_event(http2executor->peer_id, EPOLLIN | EPOLLOUT, *http2executor);

                        ctx.delayed_free(this);
                        return;
                    } else {
                        auto last_write_buffer = http_add_505_HTTP_Version_Not_Supported(read_buffer, local_address, date_str, date_str_size);
                        write_size = (size_t)(last_write_buffer - read_buffer);
                    }
                }
            } else {
                if constexpr (!use_ssl) {
                    // SSL use only ALPN
                    // Without ALPN it will continue to be HTTP 1.1
                    // We will upgrade to HTTP2 only when settings are present
                    // Or we will fail this
                    if (driver.header.upgrade_version() == http_header::VERSION::VER_2 && 
                        driver.header.fields.find(http_header::FIELD::HTTP2_Settings) != driver.header.fields.end()) {
                        // Only version 2 is supported
                        // Move to HTTP 2
                        ctx.remove_event(peer_id);

                        // below will move current structure to std::move
                        iothttp2event<use_ssl> *http2executor = new iothttp2event<use_ssl>(std::move(*this));

                        // Now we can unlock as no new call will come for this connection
                        // Also, this will free up other thread if hold up in lock
                        // As this thread is moved it is quivalent to close
                        // Even event is removed so no new event will come in
                        // We are avoiding one lock in this way
                        unlock();

                        const std::string &settings = driver.header.fields[http_header::FIELD::HTTP2_Settings];

                        // Execute all read and write
                        http2executor->upgrade(ctx, driver.header, (uint8_t *)read_buffer);

                        // Add http2executor to epoll
                        ctx.add_event(http2executor->peer_id, EPOLLIN | EPOLLOUT, *http2executor);

                        ctx.delayed_free(this);
                        return;

                    }
                }
                
                if (driver.header.method == rohit::http_header_request::METHOD::GET) {
                    auto port = local_address.port;

                    rohit::http::filemap *filemap_obj = rohit::http::webfilemap.getfilemap(port);
                    auto result = filemap_obj->cache.find(driver.header.get_path());
                    if (result == filemap_obj->cache.end()) {
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
                                {http_header::FIELD::Content_Type, file_details->content_type.ptr, file_details->content_type.size},
                            };

                            char *last_write_buffer = copy_http_response(
                                read_buffer,
                                http_header::VERSION::VER_1_1,
                                200_rc,
                                header_line,
                                file_details->content.ptr,
                                file_details->content.size
                            );

                            write_size = (size_t)(last_write_buffer - read_buffer);
                        }
                    }
                }
                else {
                    auto last_write_buffer = http_add_405_Method_Not_Allowed(read_buffer, local_address, date_str, date_str_size);
                    write_size = (size_t)(last_write_buffer - read_buffer);
                }
            }

            uint8_t *write_buffer = new uint8_t[write_size];
            std::copy(read_buffer, read_buffer + write_size, write_buffer);
            push_write(write_buffer, write_size);
            write_all(ctx);
            break;
        }

        case state_t::SOCKET_PEER_WRITE: {
            write_all(ctx);
            break;
        }

    }

    unlock();
} // void iothttpevent<use_ssl>::execute(thread_context &ctx, const uint32_t event)

// Non SSL connection will be coming from HTTP 1.1 even
// This is first request finally migrated to HTTP 2 request
// read_buffer will  not contain connection_preface
template <bool use_ssl>
void iothttp2event<use_ssl>::process_read_buffer(
            thread_context &ctx, uint8_t *read_buffer, const size_t read_buffer_size) {
    // HTTP2 on TLS require ALPN support
    auto local_address = peer_id.get_local_ipv6_addr();

    std::time_t now_time = std::time(0);   // get time now
    std::tm* now_tm = std::gmtime(&now_time);
    char date_str[config::max_date_string_size];
    size_t date_str_size = strftime(date_str, config::max_date_string_size, "%a, %d %b %Y %H:%M:%S %Z", now_tm) + 1;



    /* uint8_t *write_buffer = new uint8_t[write_size];
    std::copy(read_buffer, read_buffer + write_size, write_buffer);
    push_write({write_buffer, write_size});
    write_all(ctx); */
}

// Upgrade is for non SSL only
// HTTP 1.1 for connection without prior knowledge will call this
template <bool use_ssl>
void iothttp2event<use_ssl>::upgrade(
            thread_context &ctx, const http_header_request &header, uint8_t *read_buffer) {
    // HTTP2 on TLS require ALPN support
    auto local_address = peer_id.get_local_ipv6_addr();

    std::time_t now_time = std::time(0);   // get time now
    std::tm* now_tm = std::gmtime(&now_time);
    char date_str[config::max_date_string_size];
    size_t date_str_size = strftime(date_str, config::max_date_string_size, "%a, %d %b %Y %H:%M:%S %Z", now_tm) + 1;

    // Setting has to be present for this call
    // Caller has to check this
    std::string setting = header.fields.at(http_header::FIELD::HTTP2_Settings);    
    peer_settings.parse_base64_frame((uint8_t *)setting.c_str(), setting.size());


    /* uint8_t *write_buffer = new uint8_t[write_size];
    std::copy(read_buffer, read_buffer + write_size, write_buffer);
    push_write({write_buffer, write_size});
    write_all(ctx); */
}

template <bool use_ssl>
void iothttp2event<use_ssl>::process_request(
            thread_context &ctx,
            rohit::http::v2::request &request,
            uint8_t *pwrite_end) {
    // Process request
    // Date is used by all hence it is created here
    std::time_t now_time = std::time(0);   // get time now
    std::tm* now_tm = std::gmtime(&now_time);
    uint8_t date_str[config::max_date_string_size];
    size_t date_str_size = strftime((char *)date_str, config::max_date_string_size, "%a, %d %b %Y %H:%M:%S %Z", now_tm) + 1;

    auto local_address = peer_id.get_local_ipv6_addr();

    auto *pheader = request.get_first_header();
    while(pheader) {
        switch(pheader->method) {
        case rohit::http_header_request::METHOD::GET: {
            auto port = local_address.port;

            rohit::http::filemap *filemap_obj = rohit::http::webfilemap.getfilemap(port);
            auto result = filemap_obj->cache.find(pheader->get_path());
            if (result == filemap_obj->cache.end()) {
                pwrite_end = http2_add_404_Not_Found(pwrite_end, request, *pheader, local_address, date_str, date_str_size);
            } else {
                auto file_details = result->second;

                if (pheader->match_etag(file_details->etags, file_details->etags_size)) {
                    rohit::http::v2::frame *pframe = (rohit::http::v2::frame *)pwrite_end;
                    pwrite_end += sizeof(rohit::http::v2::frame);
                    pwrite_end = request.copy_http_header_response<http_header::FIELD::Status, 304_rc>(pwrite_end);
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::Date, date_str, date_str_size, false);
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::Server, config::web_server_name, true);
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::ETag, (uint8_t *)file_details->etags, file_details->etags_size, true);
                    pframe->init_frame(
                            (uint32_t)(pwrite_end - (uint8_t *)pframe - sizeof(rohit::http::v2::frame)),
                            rohit::http::v2::frame::type_t::HEADERS,
                            rohit::http::v2::frame::flags_t::END_HEADERS, rohit::http::v2::frame::flags_t::END_STREAM,
                            pheader->stream_identifier);
                } else {
                    rohit::http::v2::frame *pframe = (rohit::http::v2::frame *)pwrite_end;
                    pwrite_end = request.copy_http_header_response<http_header::FIELD::Status, 200_rc>(pwrite_end);
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::Date, date_str, date_str_size, false);
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::Server, config::web_server_name, true);
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::ETag, (uint8_t *)file_details->etags, file_details->etags_size, true);
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::Cache_Control, "private, max-age=2592000", true);
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::Content_Type, "text/html", true);
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::Content_Length, file_details->content.size, true);

                    pframe->init_frame(
                            (uint32_t)(pwrite_end - (uint8_t *)pframe - sizeof(rohit::http::v2::frame)),
                            rohit::http::v2::frame::type_t::HEADERS,
                            rohit::http::v2::frame::flags_t::END_HEADERS,
                            pheader->stream_identifier);

                    // Data frame
                    pframe = (rohit::http::v2::frame *)pwrite_end;
                    pwrite_end += sizeof(rohit::http::v2::frame);
                    pframe->init_frame(
                            file_details->content.size,
                            rohit::http::v2::frame::type_t::DATA,
                            rohit::http::v2::frame::flags_t::END_STREAM,
                            pheader->stream_identifier);
                    pwrite_end = std::copy(file_details->content.ptr, file_details->content.ptr + file_details->content.size, pwrite_end);
                }
            }
            break;
        }
        default:
            pwrite_end = http2_add_405_Method_Not_Allowed(pwrite_end, request, *pheader, local_address, date_str, date_str_size);
            break;
        }

        pheader = pheader->get_next();
    }

}

template <bool use_ssl>
void iothttp2event<use_ssl>::execute(thread_context &ctx, const uint32_t event) {
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
            // TODO: This must be moved out of this function
            auto err = peer_id.accept();
            if (err == err_t::SUCCESS) {
                client_state = state_t::SOCKET_PEER_EVENT;
            } else {
                close(ctx);
            }
            break;
        }

        case state_t::SOCKET_PEER_READ: {
            constexpr size_t read_buffer_size = 65535; // max read length
            uint8_t read_buffer[read_buffer_size];
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

            rohit::http::v2::request request(dynamic_table, peer_settings);
            constexpr size_t write_buffer_size = 1024*1024*10; // 10MB Stack
            uint8_t write_buffer[write_buffer_size];
            uint8_t *pwrite_end = write_buffer;
            err_t ret = request.parse(
                                read_buffer,
                                read_buffer + read_buffer_length,
                                pwrite_end);

            if (ret != err_t::HTTP2_INITIATE_GOAWAY) {
                process_request(ctx, request, pwrite_end);
                size_t write_size = pwrite_end - write_buffer;
                uint8_t *_write_buffer = new uint8_t[write_size];
                std::copy(write_buffer, pwrite_end, _write_buffer);
                push_write(write_buffer, write_size);
                write_all(ctx);
            } else {
                size_t write_size = pwrite_end - write_buffer;
                uint8_t *_write_buffer = new uint8_t[write_size];
                std::copy(write_buffer, pwrite_end, _write_buffer);
                push_write(write_buffer, write_size);
                write_all(ctx);
                close(ctx);
            }

            break;
        }

        case state_t::SOCKET_PEER_CLOSE: {
            close(ctx);
            break;
        }

        case state_t::SOCKET_PEER_WRITE: {
            write_all(ctx);
            break;
        }

    }

    unlock();
}

} // namespace rohit