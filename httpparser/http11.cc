////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <http11.hh>
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>

namespace rohit {

const char *http_header::strVERSION[] {
#define HTTP_VERSION_ENTRY(x, y) y,
    HTTP_VERSION_LIST
#undef HTTP_VERSION_ENTRY
};

const char *http_header::strFIELD[] {
#define HTTP_FIELD_ENTRY(x, y) y,
    HTTP_FIELD_LIST
#undef HTTP_FIELD_ENTRY
};

const char *http_header_request::strMETHOD[] {
#define HTTP_METHOD_ENTRY(x, y) y,
    HTTP_METHOD_LIST
#undef HTTP_METHOD_ENTRY
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

std::ostream& operator<<(std::ostream& os, const http_header_response::CODE responseCODE) {
    return os << (int)responseCODE << " " << http_header_response::getCodestring(responseCODE);
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

} // namespace rohit