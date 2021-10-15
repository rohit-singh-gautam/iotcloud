////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/ipv6addr.hh>
#include <http2.hh>

namespace rohit {

uint8_t *http_add_404_Not_Found(
            uint8_t *const buffer,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size);

uint8_t *http_add_400_Bad_Request(
            uint8_t *const buffer,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size);

uint8_t *http_add_405_Method_Not_Allowed(
            uint8_t *const buffer,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size);

uint8_t *http_add_505_HTTP_Version_Not_Supported(
            uint8_t *const buffer,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size);

template <http_header::CODE code, size_t message_size>
uint8_t *http2_add_error_html(
    uint8_t *buffer,
    rohit::http::v2::request &request,
    rohit::http::v2::header_request &header,
    const ipv6_socket_addr_t &local_address,
    const uint8_t *date_str,
    const size_t date_str_size,
    const uint8_t (&message)[message_size])
{
    rohit::http::v2::frame *pframe = (rohit::http::v2::frame *)buffer;
    buffer += sizeof(rohit::http::v2::frame);
    buffer = request.copy_http_header_response<http_header::FIELD::Status, code>(buffer);
    buffer = request.copy_http_header_response(buffer, http_header::FIELD::Date, date_str, date_str_size, false);
    buffer = request.copy_http_header_response(buffer, http_header::FIELD::Server, config::web_server_name, true);
    buffer = request.copy_http_header_response(buffer, http_header::FIELD::Content_Type, "text/html", true);
    if constexpr (code == 400_rc || code == 505_rc)
        buffer = request.copy_http_header_response(buffer, http_header::FIELD::Connection, "close", true);
    if constexpr (code == 405_rc)
        buffer = request.copy_http_header_response(buffer, http_header::FIELD::Allow, "GET, PRI", true);

    uint8_t body[1024] = {0};
    uint8_t *last_write_body = http11_error_html<http_header::CODE::_404>(body, message, local_address);
    const size_t content_length = (size_t)(last_write_body - body);
    buffer = request.copy_http_header_response(buffer, http_header::FIELD::Content_Length, content_length, true);

    pframe->init_frame(
                (uint32_t)(buffer - (uint8_t *)pframe - sizeof(rohit::http::v2::frame)),
                rohit::http::v2::frame::type_t::HEADERS,
                rohit::http::v2::frame::flags_t::END_HEADERS,
                header.stream_identifier);

    pframe = (rohit::http::v2::frame *)buffer;
    buffer += sizeof(rohit::http::v2::frame);
    pframe->init_frame(
                content_length,
                rohit::http::v2::frame::type_t::DATA,
                rohit::http::v2::frame::flags_t::END_STREAM,
                header.stream_identifier);
    buffer = std::copy(body, last_write_body, buffer);

    return buffer;
}

inline uint8_t *http2_add_404_Not_Found(
            uint8_t * buffer,
            rohit::http::v2::request &request,
            rohit::http::v2::header_request &header,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size)
{
    const uint8_t message[] = "<p>The requested URL was not found on this server.</p>";
    return http2_add_error_html<404_rc>(
        buffer, request, header, local_address, date_str, date_str_size, message);
}

inline uint8_t *http2_add_400_Bad_Request(
            uint8_t * buffer,
            rohit::http::v2::request &request,
            rohit::http::v2::header_request &header,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size)
{
    const uint8_t message[] =
        "<p>Server was unable to parse your http request.</p>"
        "<p>Are you trying to make https request on http port?</p>";
    return http2_add_error_html<400_rc>(
        buffer, request, header, local_address, date_str, date_str_size, message);
}

inline uint8_t *http2_add_405_Method_Not_Allowed(
            uint8_t * buffer,
            rohit::http::v2::request &request,
            rohit::http::v2::header_request &header,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size)
{
    const uint8_t message[] = "<p>This method is not supported by server.</p>";
    return http2_add_error_html<405_rc>(
        buffer, request, header, local_address, date_str, date_str_size, message);
}

inline uint8_t *http_add_505_HTTP_Version_Not_Supported(
            uint8_t * buffer,
            rohit::http::v2::request &request,
            rohit::http::v2::header_request &header,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size)
{
    const uint8_t message[] = "<p>This HTTP version is not supported by server.</p>";
    return http2_add_error_html<505_rc>(
        buffer, request, header, local_address, date_str, date_str_size, message);
}

} // namespace rohit