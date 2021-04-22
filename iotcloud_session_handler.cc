// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in)

#include "iotcloud_session_handler.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/http.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/json.hpp>

#include <iostream>

using tcp = boost::asio::ip::tcp;
namespace be = boost::beast;
namespace json = boost::json; // https://www.boost.org/doc/libs/1_75_0/libs/json/doc/html/json/examples.html

void iotcloud_session_handler(tcp::socket socket) {
  auto report_error = [](be::error_code ec, char const* what) {
      std::cerr << what << ": " << ec.message() << "\n";
    };

    be::error_code ec;
    for (;;) {
      be::flat_buffer buffer;

      // Read a request
      be::http::request<be::http::string_body> request;
      be::http::read(socket, buffer, request, ec);
      if (ec == be::http::error::end_of_stream) break;
      if (ec) return report_error(ec, "read");

      // Send the response
      // Respond to any request with a "Hello World" message.
      be::http::response<be::http::string_body> response{be::http::status::ok,
                                                         request.version()};
      response.set(be::http::field::server, BOOST_BEAST_VERSION_STRING);
      response.set(be::http::field::content_type, "text/plain");
      response.keep_alive(request.keep_alive());
      std::string greeting = "Hello ";
      auto const* target = std::getenv("TARGET");
      greeting += target == nullptr ? "World" : target;
      greeting += "\n";
      response.body() = std::move(greeting);
      response.prepare_payload();
      be::http::write(socket, response, ec);
      if (ec) return report_error(ec, "write");
    }
    socket.shutdown(tcp::socket::shutdown_send, ec);
}