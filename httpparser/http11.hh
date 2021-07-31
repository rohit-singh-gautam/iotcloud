////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <unordered_map>
#include <iostream>

namespace rohit {

#define LIST_DEFINITION_END

#define HTTP_VERSION_LIST \
    HTTP_VERSION_ENTRY(VER_1_1, "HTTP/1.1") \
    HTTP_VERSION_ENTRY(VER_2, "HTTP/2") \
    LIST_DEFINITION_END

#define HTTP_FIELD_LIST \
    /* General Header */ \
    HTTP_FIELD_ENTRY(Cache_Control, "Cache-Control") \
    HTTP_FIELD_ENTRY(Connection, "Connection") \
    HTTP_FIELD_ENTRY(Date, "Date") \
    HTTP_FIELD_ENTRY(Pragma, "Pragma") \
    HTTP_FIELD_ENTRY(Trailer, "Trailer") \
    HTTP_FIELD_ENTRY(Transfer_Encoding, "Transfer-Encoding") \
    HTTP_FIELD_ENTRY(Upgrade, "Upgrade") \
    HTTP_FIELD_ENTRY(Via, "Via") \
    HTTP_FIELD_ENTRY(Warning, "Warning") \
    \
    /* Request Header */ \
    HTTP_FIELD_ENTRY(Accept, "Accept") \
    HTTP_FIELD_ENTRY(Accept_Charset, "Accept-Charset") \
    HTTP_FIELD_ENTRY(Accept_Encoding, "Accept-Encoding") \
    HTTP_FIELD_ENTRY(Accept_Language, "Accept-Language") \
    HTTP_FIELD_ENTRY(Authorization, "Authorization") \
    HTTP_FIELD_ENTRY(Expect, "Expect") \
    HTTP_FIELD_ENTRY(From, "From") \
    HTTP_FIELD_ENTRY(Host, "Host") \
    HTTP_FIELD_ENTRY(If_Match, "If-Match") \
    HTTP_FIELD_ENTRY(If_Modified_Since, "If-Modified-Since") \
    HTTP_FIELD_ENTRY(If_None_Match, "If-None-Match") \
    HTTP_FIELD_ENTRY(If_Range, "If-Range") \
    HTTP_FIELD_ENTRY(If_Unmodified_Since, "If-Unmodified-Since") \
    HTTP_FIELD_ENTRY(Max_Forwards, "Max-Forwards") \
    HTTP_FIELD_ENTRY(Proxy_Authorization, "Proxy-Authorization") \
    HTTP_FIELD_ENTRY(Range, "Range") \
    HTTP_FIELD_ENTRY(Referer, "Referer") \
    HTTP_FIELD_ENTRY(TE, "TE") \
    HTTP_FIELD_ENTRY(User_Agent, "User-Agent") \
    HTTP_FIELD_ENTRY(HTTP2_Settings, "HTTP2-Settings") \
    \
    /* Response Header */ \
    HTTP_FIELD_ENTRY(Accept_Ranges, "Accept-Ranges") \
    HTTP_FIELD_ENTRY(Age, "Age") \
    HTTP_FIELD_ENTRY(ETag, "ETag") \
    HTTP_FIELD_ENTRY(Location, "Location") \
    HTTP_FIELD_ENTRY(Proxy_Authenticate, "Proxy-Authenticate") \
    HTTP_FIELD_ENTRY(Retry_After, "Retry-After") \
    HTTP_FIELD_ENTRY(Server, "Server") \
    HTTP_FIELD_ENTRY(Vary, "Vary") \
    HTTP_FIELD_ENTRY(WWW_Authenticate, "WWW-Authenticate") \
    \
    HTTP_FIELD_ENTRY(Allow, "Allow") \
    HTTP_FIELD_ENTRY(Content_Encoding, "Content-Encoding") \
    HTTP_FIELD_ENTRY(Content_Language, "Content-Language") \
    HTTP_FIELD_ENTRY(Content_Length, "Content-Length") \
    HTTP_FIELD_ENTRY(Content_Location, "Content-Location") \
    HTTP_FIELD_ENTRY(Content_MD5, "Content-MD5") \
    HTTP_FIELD_ENTRY(Content_Range, "Content-Range") \
    HTTP_FIELD_ENTRY(Content_Type, "Content-Type") \
    HTTP_FIELD_ENTRY(Expires, "Expires") \
    HTTP_FIELD_ENTRY(Last_Modified, "Last-Modified") \
    \
    LIST_DEFINITION_END

#define HTTP_METHOD_LIST \
    HTTP_METHOD_ENTRY(OPTIONS, "OPTIONS") \
    HTTP_METHOD_ENTRY(GET, "GET") \
    HTTP_METHOD_ENTRY(HEAD, "HEAD") \
    HTTP_METHOD_ENTRY(POST, "POST") \
    HTTP_METHOD_ENTRY(PUT, "PUT") \
    HTTP_METHOD_ENTRY(DELETE, "DELETE") \
    HTTP_METHOD_ENTRY(TRACE, "TRACE") \
    HTTP_METHOD_ENTRY(CONNECT, "CONNECT") \
    LIST_DEFINITION_END

class http_header {
public:
    enum class VERSION {
#define HTTP_VERSION_ENTRY(x, y) x,
    HTTP_VERSION_LIST
#undef HTTP_VERSION_ENTRY
    };

    enum class FIELD {
#define HTTP_FIELD_ENTRY(x, y) x,
    HTTP_FIELD_LIST
#undef HTTP_FIELD_ENTRY
    };

    struct line {
        const FIELD field;
        const char *value;
        const size_t size;
        template <size_t N>
        constexpr line(const FIELD field, const char value[N]) : field(field), value(value), size(N) {}
    };

    http_header() {};
    http_header(VERSION version, const std::unordered_map<http_header::FIELD, std::string> &fields);

    VERSION version;
    std::unordered_map<FIELD, std::string> fields;

private:
    static const char *strVERSION[];
    static const char *strFIELD[];

public:
    friend std::ostream& operator<<(std::ostream& os, const VERSION httpVersion);
    friend std::ostream& operator<<(std::ostream& os, const FIELD httpField);
    friend std::ostream& operator<<(std::ostream& os, const std::pair<FIELD, std::string>& httpFieldPair);
    friend std::ostream& operator<<(std::ostream& os, const std::unordered_map<FIELD, std::string>& httpFields);
};

std::ostream& operator<<(std::ostream& os, const http_header::VERSION httpVersion);
std::ostream& operator<<(std::ostream& os, const http_header::FIELD httpField);
std::ostream& operator<<(std::ostream& os, const std::pair<http_header::FIELD, std::string>& httpFieldPair);
std::ostream& operator<<(std::ostream& os, const std::unordered_map<http_header::FIELD, std::string>& httpFields);

class http_header_request : public http_header {
public:
    using http_header::VERSION;
    using http_header::FIELD;
    enum class METHOD {
#define HTTP_METHOD_ENTRY(x, y) x,
    HTTP_METHOD_LIST
#undef HTTP_METHOD_ENTRY
    };

    METHOD method;
    std::string path;

    http_header_request() {}

private:
    static const char *strMETHOD[];

public:
    friend std::ostream& operator<<(std::ostream& os, const METHOD requestMethod);
    friend std::ostream& operator<<(std::ostream& os, const http_header_request& requestHeader);
};

std::ostream& operator<<(std::ostream& os, const http_header_request::METHOD requestMethod);
std::ostream& operator<<(std::ostream& os, const http_header_request& requestHeader);

class http_header_response : public http_header {
public:
    using http_header::VERSION;
    using http_header::FIELD;
    enum class CODE : int;

    http_header_response() {}
    http_header_response(VERSION version, CODE code, const std::unordered_map<FIELD, std::string> &fields);
    CODE code;

public:
    friend std::ostream& operator<<(std::ostream& os, const CODE responseCODE);
    friend std::ostream& operator<<(std::ostream& os, const http_header_response& responseHeader);

    static constexpr const char *getCodestring(CODE code);
    constexpr const char *getCodestring() const { return http_header_response::getCodestring(code); };
};

// HTTP Response Code (HRC)
constexpr http_header_response::CODE operator"" _rc(unsigned long long code) {
   return (http_header_response::CODE)code;
}

constexpr const char *http_header_response::getCodestring(http_header_response::CODE code) {
    switch(code) {
// Informational 1xx
    case 100_rc: return "Continue";
    case 101_rc: return "Switching Protocols";

// Successful 2xx
    case 200_rc: return "OK";
    case 201_rc: return "Created";
    case 202_rc: return "Accepted";
    case 203_rc: return "Non-Authoritative Information";
    case 204_rc: return "No Content";
    case 205_rc: return "Reset Content";
    case 206_rc: return "Partial Content";

// Redirection 3xx
    case 300_rc: return "Multiple Choices";
    case 301_rc: return "Moved Permanently";
    case 302_rc: return "Found";
    case 303_rc: return "See Other";
    case 304_rc: return "Not Modified";
    case 305_rc: return "Use Proxy";
// 306 is unused
    case 307_rc: return "Temporary Redirect";

// Client Error 4xx
    case 400_rc: return "Bad Request";
    case 401_rc: return "Unauthorized";
    case 402_rc: return "Payment Required";
    case 403_rc: return "Forbidden";
    case 404_rc: return "Not Found";
    case 405_rc: return "Method Not Allowed";
    case 406_rc: return "Not Acceptable";
    case 407_rc: return "Proxy Authentication Required";
    case 408_rc: return "Request Timeout";
    case 409_rc: return "Conflict";
    case 410_rc: return "Gone";
    case 411_rc: return "Length Required";
    case 412_rc: return "Precondition Failed";
    case 413_rc: return "Request Entity Too Large";
    case 414_rc: return "Request-URI Too Long";
    case 415_rc: return "Unsupported Media Type";
    case 416_rc: return "Requested Range Not Satisfiable";
    case 417_rc: return "Expectation Failed";

// Server Error 5xx
    case 500_rc: return "Internal Server Error";
    case 501_rc: return "Not Implemented";
    case 502_rc: return "Bad Gateway";
    case 503_rc: return "Service Unavailable";
    case 504_rc: return "Gateway Timeout";
    case 505_rc: return "HTTP Version Not Supported";
    default: return "Unknown";
    }
}

std::ostream& operator<<(std::ostream& os, const http_header_response::CODE responseCODE);

std::ostream& operator<<(std::ostream& os, const http_header_response& responseHeader);

class http_response : public http_header_response {
public:
    using http_header::VERSION;
    using http_header::FIELD;
    using http_header_response::CODE;

    http_response(
        VERSION version, 
        CODE code, 
        const std::unordered_map<FIELD, std::string> &fields,
        std::string body);

    std::string body;

    void addMD5();

public:
    friend std::ostream& operator<<(std::ostream& os, const http_response& responseHeader);
};

std::ostream& operator<<(std::ostream& os, const http_response& responseHeader);

} // namespace rohit
