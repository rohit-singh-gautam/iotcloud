////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <http2.hh>

namespace rohit::http::v2 {

std::ostream& operator<<(std::ostream& os, const header_request &header_request) {
    const http_header_request &http11request = header_request;

    os << "Stream Identifier: " << header_request.stream_identifier << std::endl
        << "Weight: " << header_request.weight << std::endl;
    
    if (header_request.error != frame::error_t::NO_ERROR) {
        os << "Error: " << header_request << std::endl;
    }

    return os << header_request;
}

} // namespace rohit::http::v2