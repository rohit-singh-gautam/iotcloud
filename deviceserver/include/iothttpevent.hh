/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// This program is free software: you can redistribute it and/or modify it under the terms //
// of the GNU General Public License as published by the Free Software Foundation, either  //
// version 3 of the License, or (at your option) any later version.                        //
//                                                                                         //
// This program is distributed in the hope that it will be useful, but WITHOUT ANY         //
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A         //
// PARTICULAR PURPOSE. See the GNU General Public License for more details.                //
//                                                                                         //
// You should have received a copy of the GNU General Public License along with this       //
// program. If not, see <https://www.gnu.org/licenses/>.                                   //
/////////////////////////////////////////////////////////////////////////////////////////////

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

class iothttpsslevent : public serverpeerevent<true> {
public:
    static constexpr bool movable = true;

private:
    using serverpeerevent<true>::peer_id;
    using serverpeerevent<true>::enter_loop;
    using serverpeerevent<true>::exit_loop;
    using serverpeerevent<true>::client_state;
    using serverpeerevent<true>::write_queue;

public:
    inline iothttpsslevent(socket_ssl_t &peer_id) : serverpeerevent<true>(peer_id) { }

    void execute(thread_context &ctx) override;

    using serverpeerevent<true>::write_all;

    using serverpeerevent<true>::close;
};

template <bool use_ssl>
class iothttpevent : public serverpeerevent<use_ssl> {
public:
    static constexpr bool movable = true;

protected:
    using serverpeerevent<use_ssl>::peer_id;
    using serverpeerevent<use_ssl>::enter_loop;
    using serverpeerevent<use_ssl>::exit_loop;
    using serverpeerevent<use_ssl>::client_state;
    using serverpeerevent<use_ssl>::write_queue;

    using serverpeerevent_base::push_write;
    using serverpeerevent_base::pop_write;
    using serverpeerevent_base::get_write_buffer;
    using serverpeerevent_base::is_write_left;

    friend class iothttpsslevent;

public:
    inline iothttpevent(socket_variant_t<use_ssl>::type &peer_id) : serverpeerevent<use_ssl>(peer_id) { }
    inline iothttpevent(iothttpevent<use_ssl> &&http11event) : serverpeerevent<use_ssl>(std::move(http11event)) { }
    inline iothttpevent(iothttpsslevent &&sslevent) : serverpeerevent<use_ssl>(std::move(sslevent)) {
        static_assert(use_ssl == true, "Only SSL event allowed, this function is for ALPN");
    }

    using serverpeerevent<use_ssl>::write_all;

    void read_helper(thread_context &ctx);

    void execute(thread_context &ctx) override;

    using serverpeerevent<use_ssl>::close;
};

template <bool use_ssl>
class iothttp2event : public iothttpevent<use_ssl> {
public:
    static constexpr bool movable = false;

protected:
    using serverpeerevent<use_ssl>::peer_id;
    using serverpeerevent<use_ssl>::enter_loop;
    using serverpeerevent<use_ssl>::exit_loop;
    using serverpeerevent<use_ssl>::client_state;
    using serverpeerevent<use_ssl>::write_queue;

    using serverpeerevent_base::push_write;
    using serverpeerevent_base::pop_write;
    using serverpeerevent_base::get_write_buffer;
    using serverpeerevent_base::is_write_left;

    rohit::http::v2::dynamic_table_t dynamic_table;
    rohit::http::v2::settings_store peer_settings;

    friend class iothttpsslevent;
    friend class iothttpevent<use_ssl>;

public:
    inline iothttp2event(socket_variant_t<use_ssl>::type &&peer_id) : iothttpevent<use_ssl>(peer_id) { }
    inline iothttp2event(iothttpsslevent &&sslevent) : iothttpevent<use_ssl>(std::move(sslevent)) { }
    inline iothttp2event(iothttpevent<use_ssl> &&http11event) : iothttpevent<use_ssl>(std::move(http11event)) { }

    using serverpeerevent<use_ssl>::write_all;

    void process_request(
                thread_context &ctx,
                rohit::http::v2::request &request);

    template <bool moved>
    void process_read_buffer(thread_context &ctx, uint8_t *read_buffer, const size_t read_buffer_size);

    void upgrade(thread_context &ctx, http_header_request &header);

    template <state_t state>
    void read_helper(thread_context &ctx);

    void execute(thread_context &ctx) override;

    using serverpeerevent<use_ssl>::close;
};

// Non SSL connection will be coming from HTTP 1.1 even
// This is first request finally migrated to HTTP 2 request
// read_buffer will  not contain connection_preface
template <bool use_ssl>
template <bool first_frame>
void iothttp2event<use_ssl>::process_read_buffer(
            thread_context &ctx, uint8_t *read_buffer, const size_t read_buffer_size) {
    // HTTP2 on TLS require ALPN support
    // This function is not valid for TLS
    rohit::http::v2::request request(dynamic_table, peer_settings);
    uint8_t *const write_buffer = ctx.write_buffer;
    uint8_t *pwrite_end = write_buffer;

    if constexpr (first_frame) {
        // First frame in read buffer must be settings
        auto pframe = (rohit::http::v2::frame *)read_buffer;
        if (pframe->get_type() != rohit::http::v2::frame::type_t::SETTINGS) {
            // Initiate GOAWAY
            pwrite_end = rohit::http::v2::goaway::add_frame(
                                write_buffer, 1, // No frame has been created hence max frame is 1
                                rohit::http::v2::frame::error_t::PROTOCOL_ERROR,
                                "SETTINGS expected");

            size_t write_size = pwrite_end - write_buffer;
            uint8_t *_write_buffer = new uint8_t[write_size];
            std::copy(write_buffer, pwrite_end, _write_buffer);
            push_write(_write_buffer, write_size);
            write_all(ctx);
            close(ctx);
            return;
        }

        pwrite_end = rohit::http::v2::settings::add_frame(
                        pwrite_end,
                        rohit::http::v2::settings::identifier_t::SETTINGS_ENABLE_PUSH, 0,
                        rohit::http::v2::settings::identifier_t::SETTINGS_MAX_CONCURRENT_STREAMS, 10,
                        rohit::http::v2::settings::identifier_t::SETTINGS_INITIAL_WINDOW_SIZE, 1048576,
                        rohit::http::v2::settings::identifier_t::SETTINGS_HEADER_TABLE_SIZE, 2048);
    }

    err_t ret = request.parse(
                        read_buffer,
                        read_buffer + read_buffer_size,
                        pwrite_end);

    if (write_buffer != pwrite_end) {
        size_t write_size = pwrite_end - write_buffer;
        uint8_t *_write_buffer = new uint8_t[write_size];
        std::copy(write_buffer, pwrite_end, _write_buffer);
        push_write(_write_buffer, write_size);
    }

    if (ret != err_t::HTTP2_INITIATE_GOAWAY) {
        process_request(ctx, request);
        write_all(ctx);
    } else {
        close(ctx);
    }
}

template <bool use_ssl>
void iothttpevent<use_ssl>::read_helper(thread_context &ctx) {
    constexpr size_t read_buffer_size = thread_context::buffer_size;
    auto read_buffer = ctx.read_buffer;
    size_t read_buffer_length;

    auto err = peer_id.read(read_buffer, read_buffer_size, read_buffer_length);

    if (err == err_t::SOCKET_RETRY) {
        // No state change required
        return;
    }
    if (isFailure(err)) {
        log<log_t::HTTP_EVENT_SERVER_READ_FAILED>(err);
        return;
    }

    if (read_buffer_length == 0) {
        // No data indication that wait
        return;
    }

    std::string request_string((char *)read_buffer, read_buffer_length);

    http11driver driver;
    auto parserret = driver.parse(request_string);

    // Date is used by all hence it is created here
    std::time_t now_time = std::time(0);   // get time now
    std::tm* now_tm = std::gmtime(&now_time);
    uint8_t date_str[config::max_date_string_size];
    size_t date_str_size = strftime((char *)date_str, config::max_date_string_size, "%a, %d %b %Y %H:%M:%S %Z", now_tm) + 1;

    auto write_buffer = ctx.write_buffer;

    auto local_address = peer_id.get_local_ipv6_addr();
    size_t write_size = 0;
    if (parserret != err_t::SUCCESS) {
        auto last_write_buffer = http_add_400_Bad_Request(write_buffer, local_address, date_str, date_str_size);
        write_size = (size_t)(last_write_buffer - write_buffer);
    } if (driver.header.method == rohit::http_header_request::METHOD::PRI) {
        if constexpr(use_ssl) {
            // Reply in HTTP 1.1 only
            auto last_write_buffer = http_add_400_Bad_Request(write_buffer, local_address, date_str, date_str_size);
            write_size = (size_t)(last_write_buffer - write_buffer);
        } else {
            if (driver.header.version == http_header::VERSION::VER_2) {                
                // Move to HTTP 2
                ctx.remove_event(peer_id);

                // below will move current structure to std::move
                iothttp2event<use_ssl> *http2executor = new iothttp2event<use_ssl>(std::move(*this));

                http2executor->enter_loop();

                // Add http2executor to epoll
                ctx.add_event(http2executor->peer_id, EPOLLIN | EPOLLOUT, http2executor);

                // Execute all read and write
                auto new_read_buffer = read_buffer + rohit::http::v2::connection_preface_size;
                auto new_read_buffer_length = read_buffer_length - rohit::http::v2::connection_preface_size;

                if (new_read_buffer_length == 0) {
                    http2executor->client_state = state_t::HTTP2_FIRST_FRAME;
                } else {
                    http2executor->template process_read_buffer<true>(ctx, new_read_buffer, new_read_buffer_length);
                }

                // This is important as we may have missed few events
                http2executor->execute_protector_noenter(ctx);

                ctx.delayed_free(this);
                // No need to exit loop this will be freed anyway
                return;
            } else {
                auto last_write_buffer = http_add_505_HTTP_Version_Not_Supported(write_buffer, local_address, date_str, date_str_size);
                write_size = (size_t)(last_write_buffer - write_buffer);
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

                http2executor->enter_loop();

                // Add http2executor to epoll
                ctx.add_event(http2executor->peer_id, EPOLLIN | EPOLLOUT, http2executor);   

                // Execute all read and write
                http2executor->upgrade(ctx, driver.header);

                // This is important as we may have missed few events
                http2executor->execute_protector_noenter(ctx);

                ctx.delayed_free(this);
                return;
            }
        }
        
        if (driver.header.method == rohit::http_header_request::METHOD::GET) {
            auto port = local_address.port;

            rohit::http::filemap *filemap_obj = rohit::http::webfilemap.getfilemap(port);
            auto result = filemap_obj->cache.find(driver.header.get_path());
            if (result == filemap_obj->cache.end()) {
                auto last_write_buffer = http_add_404_Not_Found(write_buffer, local_address, date_str, date_str_size);
                write_size = (size_t)(last_write_buffer - write_buffer);
            } else {
                auto file_details = result->second;

                if (driver.header.match_etag(file_details->etags, file_details->etags_size)) {
                    const http_header_line header_line[] = {
                        {http_header::FIELD::Date, date_str, date_str_size},
                        {http_header::FIELD::Server, config::web_server_name},
                        {http_header::FIELD::ETag, file_details->etags, file_details->etags_size},
                    };
                    auto *last_write_buffer = copy_http_header_response(
                        write_buffer,
                        http_header::VERSION::VER_1_1,
                        304_rc,
                        header_line
                    );

                    *last_write_buffer++ = '\n';

                    write_size = (size_t)(last_write_buffer - write_buffer);
                } else {
                    const http_header_line header_line[] = {
                        {http_header::FIELD::Date, date_str, date_str_size},
                        {http_header::FIELD::Server, config::web_server_name},
                        {http_header::FIELD::Cache_Control, "private, max-age=2592000"},
                        {http_header::FIELD::ETag, file_details->etags, rohit::http::file_info::etags_size},
                        {http_header::FIELD::Content_Type, file_details->content_type.ptr, file_details->content_type.size},
                    };

                    auto last_write_buffer = copy_http_header_response(
                        write_buffer,
                        http_header::VERSION::VER_1_1,
                        200_rc,
                        header_line
                    );

                    last_write_buffer = copy_http_response_content_length(last_write_buffer, file_details->content.size);

                    const auto write_size_header = (size_t)(last_write_buffer - write_buffer);

                    auto _write_buffer = new uint8_t[write_size_header + 2 + file_details->content.size];
                    last_write_buffer = std::copy(write_buffer, write_buffer + write_size_header, _write_buffer);
                    *last_write_buffer++ = '\r';
                    *last_write_buffer++ = '\n';
                    last_write_buffer = std::copy(
                                            file_details->content.begin(),
                                            file_details->content.end(),
                                            last_write_buffer);

                    const auto write_size_full = (size_t)(last_write_buffer - _write_buffer);
                    push_write(_write_buffer, write_size_full);
                }
            }
        }
        else {
            auto last_write_buffer = http_add_405_Method_Not_Allowed(write_buffer, local_address, date_str, date_str_size);
            write_size = (size_t)(last_write_buffer - write_buffer);
        }
    }

    if (write_size != 0) {
        auto _write_buffer = new uint8_t[write_size];
        std::copy(write_buffer, write_buffer + write_size, _write_buffer);
        push_write(_write_buffer, write_size);
    }
    write_all(ctx);

    // Tail recurssion
    read_helper(ctx);
}

template <bool use_ssl>
void iothttpevent<use_ssl>::execute(thread_context &ctx) {
    switch (client_state) {
        case state_t::SOCKET_PEER_CLOSE: {
            // This is server initiated close
            // Socket has to be closed by server
            // SSL may require it to call again
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

        case state_t::SERVEREVENT_MOVED: {
            // This event is move to HTTP 2.0
            // We must never reach here
            log<log_t::EVENT_SERVER_MOVED_ENTERED>();
            break;
        }

        case state_t::SOCKET_PEER_CLOSED:
            // This has to be ignored
            break;

        default:
            // We must never reach here
            log<log_t::EVENT_SERVER_UNKNOWN_STATE>(client_state);
            break;
    }
} // void iothttpevent<use_ssl>::execute(thread_context &ctx)

// Upgrade is for non TLS only
// HTTP 1.1 for connection without prior knowledge will call this
template <bool use_ssl>
void iothttp2event<use_ssl>::upgrade(
            thread_context &ctx, http_header_request &header) {
    // HTTP2 on TLS require ALPN support
    // Hence this function will never be called for TLS

    // Setting has to be present for this call
    // Caller has to check this
    std::string setting = header.fields.at(http_header::FIELD::HTTP2_Settings);    
    peer_settings.parse_base64_frame((uint8_t *)setting.c_str(), setting.size());

    constexpr size_t write_buffer_size = ctx.buffer_size;
    auto write_buffer = ctx.write_buffer;
    auto pwrite_end = write_buffer;

    // Adding upgrade packet
    const http_header_line header_line[] = {
        {http_header::FIELD::Connection, "Upgrade"},
        {http_header::FIELD::Upgrade, "h2c"},
    };

    pwrite_end = copy_http_header_response(
        pwrite_end,
        http_header::VERSION::VER_1_1,
        101_rc,
        header_line
    );

    // Additional newline before http2
    *pwrite_end++ = '\r';
    *pwrite_end++ = '\n';

    pwrite_end = rohit::http::v2::settings::add_frame(
                        pwrite_end,
                        rohit::http::v2::settings::identifier_t::SETTINGS_ENABLE_PUSH, 0,
                        rohit::http::v2::settings::identifier_t::SETTINGS_MAX_CONCURRENT_STREAMS, 100,
                        rohit::http::v2::settings::identifier_t::SETTINGS_INITIAL_WINDOW_SIZE, 1048576,
                        rohit::http::v2::settings::identifier_t::SETTINGS_HEADER_TABLE_SIZE, 2048);

    pwrite_end = rohit::http::v2::settings::add_ack_frame(pwrite_end);
    size_t write_size = pwrite_end - write_buffer;
    uint8_t *_write_buffer = new uint8_t[write_size];
    std::copy(write_buffer, pwrite_end, _write_buffer);
    push_write(_write_buffer, write_size);

    rohit::http::v2::request request(dynamic_table, peer_settings, std::move(header));
    process_request(ctx, request);
    write_all(ctx);

    client_state = state_t::HTTP2_NEXT_MAGIC;
}

template <bool use_ssl>
void iothttp2event<use_ssl>::process_request(
            thread_context &ctx,
            rohit::http::v2::request &request) {
    // Process request
    // Date is used by all hence it is created here
    std::time_t now_time = std::time(0);   // get time now
    std::tm* now_tm = std::gmtime(&now_time);
    uint8_t date_str[config::max_date_string_size];
    size_t date_str_size = strftime((char *)date_str, config::max_date_string_size, "%a, %d %b %Y %H:%M:%S %Z", now_tm) + 1;

    auto local_address = peer_id.get_local_ipv6_addr();

    uint8_t *const write_buffer = ctx.write_buffer;
    uint8_t *pwrite_end = write_buffer;

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
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::Date, date_str, date_str_size, true);
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::Server, config::web_server_name, true);
                    pwrite_end = request.copy_http_header_response(pwrite_end, http_header::FIELD::ETag, (uint8_t *)file_details->etags, file_details->etags_size, true);
                    pframe->init_frame(
                            (uint32_t)(pwrite_end - (uint8_t *)pframe - sizeof(rohit::http::v2::frame)),
                            rohit::http::v2::frame::type_t::HEADERS,
                            rohit::http::v2::frame::flags_t::END_HEADERS, rohit::http::v2::frame::flags_t::END_STREAM,
                            pheader->stream_identifier);
                } else {
                    rohit::http::v2::frame *pframe = (rohit::http::v2::frame *)pwrite_end;
                    pwrite_end += sizeof(rohit::http::v2::frame);
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

                    if (write_buffer != pwrite_end) {
                        size_t write_size = pwrite_end - write_buffer;
                        uint8_t *_write_buffer = new uint8_t[write_size];
                        std::copy(write_buffer, pwrite_end, _write_buffer);
                        push_write(_write_buffer, write_size);
                        pwrite_end = write_buffer;
                    }

                    const uint8_t *data_ptr = (uint8_t *)file_details->content.ptr;
                    size_t data_size = file_details->content.size;
                    constexpr size_t frame_size = ctx.buffer_size; // 16 KB
                    size_t write_size;
                    uint8_t *_write_buffer;
                    while(data_size + sizeof(rohit::http::v2::frame) > frame_size) {
                        pframe = (rohit::http::v2::frame *)pwrite_end;
                        pwrite_end += sizeof(rohit::http::v2::frame);
                        const size_t current_size = frame_size - sizeof(rohit::http::v2::frame);
                        pframe->init_frame(
                                current_size,
                                rohit::http::v2::frame::type_t::DATA,
                                rohit::http::v2::frame::flags_t::NONE,
                                pheader->stream_identifier);
                        pwrite_end = std::copy(data_ptr, data_ptr + current_size, pwrite_end);

                        write_size = pwrite_end - write_buffer;
                        _write_buffer = new uint8_t[write_size];
                        std::copy(write_buffer, pwrite_end, _write_buffer);
                        push_write(_write_buffer, write_size);
                        pwrite_end = write_buffer;

                        data_size -= current_size;
                        data_ptr += current_size;
                    }

                    // Data frame
                    pframe = (rohit::http::v2::frame *)pwrite_end;
                    pwrite_end += sizeof(rohit::http::v2::frame);
                    pframe->init_frame(
                            data_size,
                            rohit::http::v2::frame::type_t::DATA,
                            rohit::http::v2::frame::flags_t::END_STREAM,
                            pheader->stream_identifier);
                    pwrite_end = std::copy(data_ptr, data_ptr + data_size, pwrite_end);

                    write_size = pwrite_end - write_buffer;
                    _write_buffer = new uint8_t[write_size];
                    std::copy(write_buffer, pwrite_end, _write_buffer);
                    push_write(_write_buffer, write_size);
                    pwrite_end = write_buffer;
                }
            }
            break;
        }
        default:
            pwrite_end = http2_add_405_Method_Not_Allowed(pwrite_end, request, *pheader, local_address, date_str, date_str_size);
            break;
        }

        if (write_buffer != pwrite_end) {
            size_t write_size = pwrite_end - write_buffer;
            uint8_t *_write_buffer = new uint8_t[write_size];
            std::copy(write_buffer, pwrite_end, _write_buffer);
            push_write(_write_buffer, write_size);
            pwrite_end = write_buffer;
        }

        pheader = pheader->get_next();
    }
}

template <bool use_ssl>
template <state_t state>
void iothttp2event<use_ssl>::read_helper(thread_context &ctx) {
    constexpr size_t read_buffer_size = ctx.buffer_size;
    uint8_t  *read_buffer = ctx.read_buffer;
    size_t read_buffer_length;

    auto err = peer_id.read(read_buffer, read_buffer_size, read_buffer_length);

    if (err == err_t::SOCKET_RETRY) {
        // No state change required
        return;
    }
    if (isFailure(err)) {
        log<log_t::HTTP2_EVENT_SERVER_READ_FAILED>(err);
        return;
    }

    if (read_buffer_length == 0) {
        // No data indication that wait
        return;
    }

    if constexpr (state == state_t::HTTP2_NEXT_MAGIC) {
        std::string str_data((char *)read_buffer, rohit::http::v2::connection_preface_size);

        if (read_buffer_length < rohit::http::v2::connection_preface_size ||
            strncmp((char *)read_buffer, rohit::http::v2::connection_preface, rohit::http::v2::connection_preface_size))
        {
            // This is bad request
            close(ctx);
            return;
        }
        
        auto new_read_buffer = read_buffer + rohit::http::v2::connection_preface_size;
        auto new_read_buffer_length = read_buffer_length - rohit::http::v2::connection_preface_size;

        if (new_read_buffer_length == 0) {
            client_state = state_t::HTTP2_FIRST_FRAME;
            return;
        } else {
            client_state = state_t::SOCKET_PEER_EVENT;
            process_read_buffer<true>(ctx, new_read_buffer, new_read_buffer_length);
        }
    } else {
        process_read_buffer<state == state_t::HTTP2_FIRST_FRAME>(ctx, read_buffer, read_buffer_length);
    }

    if constexpr (use_ssl) if (!peer_id.is_closed()) read_helper<state_t::SOCKET_PEER_READ>(ctx);
}

template <bool use_ssl>
void iothttp2event<use_ssl>::execute(thread_context &ctx) {
    switch (client_state) {
        case state_t::HTTP2_NEXT_MAGIC: {
            read_helper<state_t::HTTP2_NEXT_MAGIC>(ctx);
            break;
        }

        case state_t::HTTP2_FIRST_FRAME: {
            read_helper<state_t::HTTP2_FIRST_FRAME>(ctx);
            break;
        }

        case state_t::SOCKET_PEER_EVENT:
        case state_t::SOCKET_PEER_READ: {
            read_helper<state_t::SOCKET_PEER_READ>(ctx);
            break;
        }

        case state_t::SOCKET_PEER_CLOSE: {
            close(ctx);
            break;
        }
        case state_t::SOCKET_PEER_CLOSED:
            // This has to be ignored
            break;

        case state_t::SOCKET_PEER_WRITE: {
            write_all(ctx);
            read_helper<state_t::SOCKET_PEER_READ>(ctx);
            break;
        }

        default:
            // We must never reach here
            log<log_t::EVENT_SERVER_UNKNOWN_STATE>(client_state);
            break;
    }
}

} // namespace rohit