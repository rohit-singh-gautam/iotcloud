// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in)
#include "http11.hh"
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>

const std::string http_header::strVERSION[] {
    "HTTP/1.1",
    "HTTP/2",
    "HTTP/3",
};

const std::string http_header::strFIELD[] {
    // General Header
    "Cache-Control",
    "Connection",
    "Date",
    "Pragma",
    "Trailer",
    "Transfer-Encoding",
    "Upgrade",
    "Via",
    "Warning",

    // Request Header
    "Accept",
    "Accept-Charset",
    "Accept-Encoding",
    "Accept-Language",
    "Authorization",
    "Expect",
    "From",
    "Host",
    "If-Match",
    "If-Modified-Since",
    "If-None-Match",
    "If-Range",
    "If-Unmodified-Since",
    "Max-Forwards",
    "Proxy-Authorization",
    "Range",
    "Referer",
    "TE",
    "User-Agent",
    "HTTP2-Settings",

    // Response Header
    "Accept-Ranges",
    "Age",
    "ETag",
    "Location",
    "Proxy-Authenticate",
    "Retry-After",
    "Server",
    "Vary",
    "WWW-Authenticate",

    // Entity Header
    "Allow",
    "Content-Encoding",
    "Content-Language",
    "Content-Length",
    "Content-Location",
    "Content-MD5",
    "Content-Range",
    "Content-Type",
    "Expires",
    "Last-Modified",

};

const std::string http_header_request::strMETHOD[] {
    "OPTIONS",
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "TRACE",
    "CONNECT",
};

std::ostream& operator<<(std::ostream& os, const http_header::VERSION requestVersion) {
    return os << http_header::strVERSION[(int)requestVersion];
}

std::ostream& operator<<(std::ostream& os, const http_header::FIELD requestField) {
    return os << http_header::strFIELD[(int)requestField];
}

std::ostream& operator<<(std::ostream& os, const std::pair<http_header::FIELD, std::string>& httpFieldPair) {
    return os << httpFieldPair.first << ": " << httpFieldPair.second;
}

std::ostream& operator<<(std::ostream& os, const std::unordered_map<http_header::FIELD, std::string>& httpFields) {
    for(auto httpFieldPair: httpFields) {
        os << httpFieldPair << "\n";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const http_header_request::METHOD requestMethod) {
    return os << http_header_request::strMETHOD[(int)requestMethod];
}

std::ostream& operator<<(std::ostream& os, const http_header_request& requestHeader) {
    return os << requestHeader.method << " " << requestHeader.path << " " << requestHeader.version << "\n"
              << requestHeader.fields << "\n";
}

const std::unordered_map<http_header_response::CODE, std::string> http_header_response::strCODE = {
    // Informational 1xx
    {100_rc, "Continue"},
    {101_rc, "Switching Protocols"},

    // Successful 2xx
    {200_rc, "OK"},
    {201_rc, "Created"},
    {202_rc, "Accepted"},
    {203_rc, "Non-Authoritative Information"},
    {204_rc, "No Content"},
    {205_rc, "Reset Content"},
    {206_rc, "Partial Content"},

    // Redirection 3xx
    {300_rc, "Multiple Choices"},
    {301_rc, "Moved Permanently"},
    {302_rc, "Found"},
    {303_rc, "See Other"},
    {304_rc, "Not Modified"},
    {305_rc, "Use Proxy"},
    // 306 is unused
    {307_rc, "Temporary Redirect"},

    // Client Error 4xx
    {400_rc, "Bad Request"},
    {401_rc, "Unauthorized"},
    {402_rc, "Payment Required"},
    {403_rc, "Forbidden"},
    {404_rc, "Not Found"},
    {405_rc, "Method Not Allowed"},
    {406_rc, "Not Acceptable"},
    {407_rc, "Proxy Authentication Required"},
    {408_rc, "Request Timeout"},
    {409_rc, "Conflict"},
    {410_rc, "Gone"},
    {411_rc, "Length Required"},
    {412_rc, "Precondition Failed"},
    {413_rc, "Request Entity Too Large"},
    {414_rc, "Request-URI Too Long"},
    {415_rc, "Unsupported Media Type"},
    {416_rc, "Requested Range Not Satisfiable"},
    {417_rc, "Expectation Failed"},

    // Server Error 5xx
    {500_rc, "Internal Server Error"},
    {501_rc, "Not Implemented"},
    {502_rc, "Bad Gateway"},
    {503_rc, "Service Unavailable"},
    {504_rc, "Gateway Timeout"},
    {505_rc, "HTTP Version Not Supported"},
};

std::ostream& operator<<(std::ostream& os, const http_header_response::CODE responseCODE) {
    auto strCODEPair = http_header_response::strCODE.find(responseCODE);
    if (strCODEPair == http_header_response::strCODE.end() ) {
        return os << (int)responseCODE << " Unknown";
    } else {
        return os << (int)responseCODE << " " << strCODEPair->second;
    }
}

std::ostream& operator<<(std::ostream& os, const http_header_response& responseHeader) {
    return os << responseHeader.version << " " << responseHeader.code << "\n"
              << responseHeader.fields << "\n";
}

http_header::http_header(
    http_header::VERSION version,
    const std::unordered_map<http_header::FIELD, std::string> &fields) : version(version), fields(fields) {}

http_header_response::http_header_response(
    http_header::VERSION version,
    http_header_response::CODE code,
    const std::unordered_map<http_header::FIELD, std::string> &fields) : http_header(version, fields), code(code) {}

http_response::http_response(
    VERSION version, 
    CODE code, 
    const std::unordered_map<FIELD, std::string> &fields,
    std::string body) : http_header_response(version, code, fields), body(body) {

    this->fields.insert(std::make_pair<FIELD, std::string>(FIELD::Content_Length, std::to_string(body.length())));
}

std::ostream& operator<<(std::ostream& os, const http_response& responseContent) {
    return os << responseContent.version << " " << responseContent.code << "\n"
              << responseContent.fields << "\n" << responseContent.body;
}

void http_response::addMD5() {
    unsigned char hash[MD5_DIGEST_LENGTH];    
    MD5((const unsigned char*)body.c_str(), body.length(), hash);

    std::ostringstream md5Stream;
    md5Stream << std::hex << std::setfill('0');

    for(long long c: hash) md5Stream << std::setw(2) << (long long)c;

    fields.insert(std::make_pair<FIELD, std::string>(FIELD::Content_MD5, std::move(md5Stream.str())));
}