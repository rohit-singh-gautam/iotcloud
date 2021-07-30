////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <unordered_map>
#include <iostream>

class http_header {
public:
    enum class VERSION {
        VER_1_1,
        VER_2,
        VER_3
    };

    enum class FIELD {
        // General Header
        Cache_Control, // Cache-Control
        Connection,
        Date,
        Pragma,
        Trailer,
        Transfer_Encoding, // Transfer-Encoding
        Upgrade,
        Via,
        Warning,

        // Request Header
        Accept,
        Accept_Charset, // Accept-Charset
        Accept_Encoding, // Accept-Encoding
        Accept_Language, // Accept-Language
        Authorization,
        Expect,
        From,
        Host,
        If_Match, // If-Match
        If_Modified_Since, // If-Modified-Since
        If_None_Match, // If-None-Match
        If_Range, // If-Range
        If_Unmodified_Since, // If-Unmodified-Since
        Max_Forwards, // Max-Forwards
        Proxy_Authorization, // Proxy-Authorization
        Range,
        Referer,
        TE,
        User_Agent, // User-Agent
        HTTP2_Settings, // HTTP2-Settings

        // Response Header
        Accept_Ranges, // Accept-Ranges
        Age,
        ETag,
        Location,
        Proxy_Authenticate, // Proxy-Authenticate
        Retry_After, // Retry-After
        Server,
        Vary,
        WWW_Authenticate, // WWW-Authenticate

        Allow,
        Content_Encoding, // Content-Encoding
        Content_Language, // Content-Language
        Content_Length, // Content-Length
        Content_Location, // Content-Location
        Content_MD5, // Content-MD5
        Content_Range, // Content-Range
        Content_Type, // Content-Type
        Expires,
        Last_Modified, // Last-Modified
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
        OPTIONS,
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        TRACE,
        CONNECT
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
