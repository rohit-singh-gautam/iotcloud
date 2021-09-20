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
    const char filenotfound1[] =
        "<!DOCTYPE HTML>"
        "<html><head>"
        "<title>404 Not Found</title>"
        "</head><body>"
        "<h1>Not Found</h1>"
        "<p>The requested URL was not found on this server.</p><hr>"
        "<address>" WEB_SERVER_NAME " Server at ";

    const char filenotfound2[] =
        "</address></body></html>\n";

    char *last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        404_rc,
        header_line
    );
    
    char body[1024] = {0};
    char *last_write_body = body;

    last_write_body = std::copy(filenotfound1, filenotfound1 + sizeof(filenotfound1)-1, last_write_body);
    auto length = to_string<number_case::upper, false>(local_address, last_write_body);
    last_write_body += length;
    last_write_body = std::copy(filenotfound2, filenotfound2 + sizeof(filenotfound2)-1, last_write_body);

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
    const char filenotfound1[] =
        "<!DOCTYPE HTML>"
        "<html><head>"
        "<title>400 Bad Request</title>"
        "</head><body>"
        "<h1>Bad Request</h1>"
        "<p>Server was unable to parse your http request.</p>"
        "<p>Are you trying to make https request on http port?</p><hr>"
        "<address>" WEB_SERVER_NAME " Server at ";

    const char filenotfound2[] =
        "</address></body></html>\n";

    char *last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        400_rc,
        header_line
    );
    
    char body[1024] = {0};
    char *last_write_body = body;

    last_write_body = std::copy(filenotfound1, filenotfound1 + sizeof(filenotfound1)-1, last_write_body);
    auto length = to_string<number_case::upper, false>(local_address, last_write_body);
    last_write_body += length;
    last_write_body = std::copy(filenotfound2, filenotfound2 + sizeof(filenotfound2)-1, last_write_body);

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

char *http_add_400_Bad_Request_HTTP2(
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
    const char filenotfound1[] =
        "<!DOCTYPE HTML>"
        "<html><head>"
        "<title>400 Bad Request</title>"
        "</head><body>"
        "<h1>Bad Request</h1>"
        "<p>HTTP2 on TLS requires ALPN support</p>"
        "<p>Are you trying to make http request on https port?</p><hr>"
        "<address>" WEB_SERVER_NAME " Server at ";

    const char filenotfound2[] =
        "</address></body></html>\n";

    char *last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        400_rc,
        header_line
    );
    
    char body[1024] = {0};
    char *last_write_body = body;

    last_write_body = std::copy(filenotfound1, filenotfound1 + sizeof(filenotfound1)-1, last_write_body);
    auto length = to_string<number_case::upper, false>(local_address, last_write_body);
    last_write_body += length;
    last_write_body = std::copy(filenotfound2, filenotfound2 + sizeof(filenotfound2)-1, last_write_body);

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
    const char filenotfound1[] =
        "<!DOCTYPE HTML>"
        "<html><head>"
        "<title>405 Method Not Allowed</title>"
        "</head><body>"
        "<h1>Method Not Allowed</h1>"
        "<p>This method is not supported by server.</p>"
        "<address>" WEB_SERVER_NAME " Server at ";

    const char filenotfound2[] =
        "</address></body></html>\n";

    char *last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        405_rc,
        header_line
    );
    
    char body[1024] = {0};
    char *last_write_body = body;

    last_write_body = std::copy(filenotfound1, filenotfound1 + sizeof(filenotfound1)-1, last_write_body);
    auto length = to_string<number_case::upper, false>(local_address, last_write_body);
    last_write_body += length;
    last_write_body = std::copy(filenotfound2, filenotfound2 + sizeof(filenotfound2)-1, last_write_body);

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
    const char filenotfound1[] =
        "<!DOCTYPE HTML>"
        "<html><head>"
        "<title>505 HTTP Version Not Supported</title>"
        "</head><body>"
        "<h1>HTTP Version Not Supported</h1>"
        "<p>This method is not supported by server.</p>"
        "<address>" WEB_SERVER_NAME " Server at ";

    const char filenotfound2[] =
        "</address></body></html>\n";

    char *last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        505_rc,
        header_line
    );
    
    char body[1024] = {0};
    char *last_write_body = body;

    last_write_body = std::copy(filenotfound1, filenotfound1 + sizeof(filenotfound1)-1, last_write_body);
    auto length = to_string<number_case::upper, false>(local_address, last_write_body);
    last_write_body += length;
    last_write_body = std::copy(filenotfound2, filenotfound2 + sizeof(filenotfound2)-1, last_write_body);

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