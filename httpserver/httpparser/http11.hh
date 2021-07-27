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

    http_header() {};
    http_header(VERSION version, const std::unordered_map<http_header::FIELD, std::string> &fields);

    VERSION version;
    std::unordered_map<FIELD, std::string> fields;

private:
    static const std::string strVERSION[];
    static const std::string strFIELD[];

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
    static const std::string strMETHOD[];

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

private:
    static const std::unordered_map<CODE, std::string> strCODE;

public:
    friend std::ostream& operator<<(std::ostream& os, const CODE responseCODE);
    friend std::ostream& operator<<(std::ostream& os, const http_header_response& responseHeader);
};

// HTTP Response Code (HRC)
inline constexpr http_header_response::CODE operator"" _rc(unsigned long long code) {
   return (http_header_response::CODE)code;
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
