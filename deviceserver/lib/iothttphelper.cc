////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iothttphelper.hh>
#include <http11driver.hh>
#include <iot/core/config.hh>

namespace rohit {

char *http_add_404_Not_Found(
            char *const buffer,
            const ipv6_socket_addr_t &local_address,
            const char *date_str,
            const size_t date_str_size)
{
    const http_header_line header_line[] = {
        {http_header::FIELD::Date, date_str, date_str_size},
        {http_header::FIELD::Server, config::web_server_name},
        {http_header::FIELD::Content_Type, "text/html"},
    };

    char *last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        404_rc,
        header_line
    );
    
    char body[1024] = {0};
    char *last_write_body = body;

    const char message[] =
                "<p>The requested URL was not found on this server.</p>";
    last_write_body = http11_error_html<http_header::CODE::_404>(last_write_body, message, local_address);

    char content_length[10];
    size_t content_length_size = to_string((size_t)(last_write_body - body), content_length);
    const http_header_line length_line(http_header::FIELD::Content_Length, content_length, content_length_size);
    last_write_buffer = copy_http_header_response(
        last_write_buffer,
        length_line
    );
    *last_write_buffer++ = '\n';

    last_write_buffer = std::copy(body, last_write_body, last_write_buffer);

    return last_write_buffer;
}

char *http_add_400_Bad_Request(
            char *const buffer,
            const ipv6_socket_addr_t &local_address,
            const char *date_str,
            const size_t date_str_size)
{
    const http_header_line header_line[] = {
        {http_header::FIELD::Date, date_str, date_str_size},
        {http_header::FIELD::Server, config::web_server_name},
        {http_header::FIELD::Content_Type, "text/html"},
        {http_header::FIELD::Connection, "close"},
    };

    char *last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        400_rc,
        header_line
    );
    
    char body[1024] = {0};
    char *last_write_body = body;

    const char message[] =
                "<p>Server was unable to parse your http request.</p>"
                "<p>Are you trying to make https request on http port?</p>";
    last_write_body = http11_error_html<http_header::CODE::_400>(last_write_body, message, local_address);

    char content_length[10];
    size_t content_length_size = to_string((size_t)(last_write_body - body), content_length);
    const http_header_line length_line(http_header::FIELD::Content_Length, content_length, content_length_size);
    last_write_buffer = copy_http_header_response(
        last_write_buffer,
        length_line
    );
    *last_write_buffer++ = '\n';

    last_write_buffer = std::copy(body, last_write_body, last_write_buffer);

    return last_write_buffer;
}

char *http_add_405_Method_Not_Allowed(
            char *const buffer,
            const ipv6_socket_addr_t &local_address,
            const char *date_str,
            const size_t date_str_size)
{
    const http_header_line header_line[] = {
        {http_header::FIELD::Date, date_str, date_str_size},
        {http_header::FIELD::Server, config::web_server_name},
        {http_header::FIELD::Content_Type, "text/html"},
        {http_header::FIELD::Allow, "GET, PRI"},
    };

    char *last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        405_rc,
        header_line
    );
    
    char body[1024] = {0};
    char *last_write_body = body;

    const char message[] =
                "<p>This method is not supported by server.</p>";
    last_write_body = http11_error_html<http_header::CODE::_405>(last_write_body, message, local_address);

    char content_length[10];
    size_t content_length_size = to_string((size_t)(last_write_body - body), content_length);
    const http_header_line length_line(http_header::FIELD::Content_Length, content_length, content_length_size);
    last_write_buffer = copy_http_header_response(
        last_write_buffer,
        length_line
    );
    *last_write_buffer++ = '\n';

    last_write_buffer = std::copy(body, last_write_body, last_write_buffer);

    return last_write_buffer;
}

char *http_add_505_HTTP_Version_Not_Supported(
            char *const buffer,
            const ipv6_socket_addr_t &local_address,
            const char *date_str,
            const size_t date_str_size)
{
    const http_header_line header_line[] = {
        {http_header::FIELD::Date, date_str, date_str_size},
        {http_header::FIELD::Server, config::web_server_name},
        {http_header::FIELD::Content_Type, "text/html"},
        {http_header::FIELD::Connection, "close"},
    };

    char *last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        505_rc,
        header_line
    );
    
    char body[1024] = {0};
    char *last_write_body = body;

    const char message[] =
                "<p>This HTTP version is not supported by server.</p>";
    last_write_body = http11_error_html<http_header::CODE::_505>(last_write_body, message, local_address);

    char content_length[10];
    size_t content_length_size = to_string((size_t)(last_write_body - body), content_length);
    const http_header_line length_line(http_header::FIELD::Content_Length, content_length, content_length_size);
    last_write_buffer = copy_http_header_response(
        last_write_buffer,
        length_line
    );
    *last_write_buffer++ = '\n';

    last_write_buffer = std::copy(body, last_write_body, last_write_buffer);

    return last_write_buffer;
}

} // namespace rohit