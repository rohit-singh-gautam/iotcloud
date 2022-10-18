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

#include <iothttphelper.hh>
#include <http11driver.hh>
#include <iot/core/config.hh>

namespace rohit {

uint8_t *http_add_404_Not_Found(
            uint8_t *const buffer,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size)
{
    const http_header_line header_line[] = {
        {http_header::FIELD::Date, date_str, date_str_size},
        {http_header::FIELD::Server, config::web_server_name},
        {http_header::FIELD::Content_Type, "text/html"},
    };

    auto last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        404_rc,
        header_line
    );
    
    uint8_t body[1024] = {0};
    uint8_t *last_write_body = body;

    const uint8_t message[] =
                "<p>The requested URL was not found on this server.</p>";
    last_write_body = http11_error_html<http_header::CODE::_404>(last_write_body, message, local_address);

    uint8_t content_length[10];
    size_t content_length_size = to_string((size_t)(last_write_body - body), content_length);
    const http_header_line length_line(http_header::FIELD::Content_Length, content_length, content_length_size);
    last_write_buffer = copy_http_header_response(
        last_write_buffer,
        length_line
    );
    *last_write_buffer++ = '\r';
    *last_write_buffer++ = '\n';

    last_write_buffer = std::copy(body, last_write_body, last_write_buffer);

    return last_write_buffer;
}

uint8_t *http_add_400_Bad_Request(
            uint8_t *const buffer,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size)
{
    const http_header_line header_line[] = {
        {http_header::FIELD::Date, date_str, date_str_size},
        {http_header::FIELD::Server, config::web_server_name},
        {http_header::FIELD::Content_Type, "text/html"},
        {http_header::FIELD::Connection, "close"},
    };

    uint8_t *last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        400_rc,
        header_line
    );
    
    uint8_t body[1024] = {0};
    uint8_t *last_write_body = body;

    const uint8_t message[] =
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
    *last_write_buffer++ = '\r';
    *last_write_buffer++ = '\n';

    last_write_buffer = std::copy(body, last_write_body, last_write_buffer);

    return last_write_buffer;
}

uint8_t *http_add_405_Method_Not_Allowed(
            uint8_t *const buffer,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size)
{
    const http_header_line header_line[] = {
        {http_header::FIELD::Date, date_str, date_str_size},
        {http_header::FIELD::Server, config::web_server_name},
        {http_header::FIELD::Content_Type, "text/html"},
        {http_header::FIELD::Allow, "GET, PRI"},
    };

    auto last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        405_rc,
        header_line
    );
    
    uint8_t body[1024] = {0};
    uint8_t *last_write_body = body;

    const uint8_t message[] =
                "<p>This method is not supported by server.</p>";
    last_write_body = http11_error_html<http_header::CODE::_405>(last_write_body, message, local_address);

    uint8_t content_length[10];
    size_t content_length_size = to_string((size_t)(last_write_body - body), content_length);
    const http_header_line length_line(http_header::FIELD::Content_Length, (char *)content_length, content_length_size);
    last_write_buffer = copy_http_header_response(
        last_write_buffer,
        length_line
    );
    *last_write_buffer++ = '\r';
    *last_write_buffer++ = '\n';

    last_write_buffer = std::copy(body, last_write_body, last_write_buffer);

    return last_write_buffer;
}

uint8_t *http_add_505_HTTP_Version_Not_Supported(
            uint8_t *const buffer,
            const ipv6_socket_addr_t &local_address,
            const uint8_t *date_str,
            const size_t date_str_size)
{
    const http_header_line header_line[] = {
        {http_header::FIELD::Date, date_str, date_str_size},
        {http_header::FIELD::Server, config::web_server_name},
        {http_header::FIELD::Content_Type, "text/html"},
        {http_header::FIELD::Connection, "close"},
    };

    auto last_write_buffer = copy_http_header_response(
        buffer,
        http_header::VERSION::VER_1_1,
        505_rc,
        header_line
    );
    
    uint8_t body[1024] = {0};
    uint8_t *last_write_body = body;

    const uint8_t message[] =
                "<p>This HTTP version is not supported by server.</p>";
    last_write_body = http11_error_html<http_header::CODE::_505>(last_write_body, message, local_address);

    uint8_t content_length[10];
    size_t content_length_size = to_string((size_t)(last_write_body - body), content_length);
    const http_header_line length_line(http_header::FIELD::Content_Length, (char *)content_length, content_length_size);
    last_write_buffer = copy_http_header_response(
        last_write_buffer,
        length_line
    );
    *last_write_buffer++ = '\r';
    *last_write_buffer++ = '\n';

    last_write_buffer = std::copy(body, last_write_body, last_write_buffer);

    return last_write_buffer;
}

} // namespace rohit