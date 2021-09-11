////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <http11.hh>
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>

namespace rohit {

std::ostream& operator<<(std::ostream& os, const http_header::VERSION httpVersion) {
    switch(httpVersion) {
#define HTTP_VERSION_ENTRY(x, y) case http_header::VERSION::x: os << y; break;
    HTTP_VERSION_LIST
#undef HTTP_VERSION_ENTRY
    default:
        os << "Unknown version " << (int)httpVersion;
        break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const http_header::FIELD requestField) {
    switch(requestField) {
#define HTTP_FIELD_ENTRY(x, y) case http_header::FIELD::x: os << y; break;
    HTTP_FIELD_LIST
#undef HTTP_FIELD_ENTRY
    default:
        os << "Unknown field";
        break;
    }
    return os;
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

std::ostream& operator<<(std::ostream& os, const std::pair<std::string, std::string>& httpFieldPair) {
    return os << httpFieldPair.first << ": " << httpFieldPair.second;
}

std::ostream& operator<<(std::ostream& os, const std::unordered_map<std::string, std::string>& httpFields) {
    for(auto httpFieldPair: httpFields) {
        os << httpFieldPair << "\n";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const http_header_request::METHOD requestMethod) {
    switch(requestMethod) {
#define HTTP_METHOD_ENTRY(x) case http_header_request::METHOD::x: os << #x; break;
    HTTP_METHOD_LIST
#undef HTTP_METHOD_ENTRY
    default:
        os << "Unknown Method";
        break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const http_header_request& requestHeader) {
    return os << requestHeader.method << " " << requestHeader.version << "\n"
              << requestHeader.fields << "\n";
}

std::ostream& operator<<(std::ostream& os, const http_header::CODE responseCODE) {
    return os << (int)responseCODE << " " << http_header::get_code_string(responseCODE);
}

std::ostream& operator<<(std::ostream& os, const http_header_response_status& responseHeader) {
    return os << responseHeader.version << " " << responseHeader.code << "\n";
}

std::ostream& operator<<(std::ostream& os, const http_header_response& responseHeader) {
    return os << responseHeader.version << " " << responseHeader.code << "\n"
              << responseHeader.fields << "\n";
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


bool http_header_request::match_etag(const char *etag, size_t etag_size) {
    if (etag[etag_size - 1] == '\0') --etag_size;
    const auto itr = fields.find(FIELD::If_None_Match);
    if (itr == fields.end()) return false;

    bool ignore = false;
    size_t index = 0;

    for(const auto ch: itr->second) {
        if (ch == ' ' || ch == ',' || ch == '"') {
            if (index == etag_size && !ignore) return true;
            ignore = false;
            index = 0;
            continue;
        }
        if (ignore) continue;
        if (index == etag_size) {
            ignore = true;
            continue;
        }
        if (ch != etag[index++]) ignore = true;
    }
    if (index == etag_size && !ignore) return true;

    return false;
}

const std::unordered_map<std::string, http_header::FIELD> http_header::field_map = {
#define HTTP_FIELD_ENTRY(x, y) {y, http_header::FIELD::x},
    HTTP_FIELD_LIST
#undef HTTP_FIELD_ENTRY
};

const std::unordered_map<std::string, http_header_request::METHOD> http_header_request::method_map = {
#define HTTP_METHOD_ENTRY(x) {#x, http_header_request::METHOD::x},
    HTTP_METHOD_LIST
#undef HTTP_METHOD_ENTRY
};

} // namespace rohit