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

#include <http11.hh>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>

namespace rohit {

std::ostream& operator<<(std::ostream& os, const http_header::VERSION httpVersion) {
    switch(httpVersion) {
#define HTTP_VERSION_ENTRY(x, y) case http_header::VERSION::x: os << y; break;
    HTTP_VERSION_LIST
#undef HTTP_VERSION_ENTRY
    default:
        os << "Unknown version " << static_cast<int>(httpVersion);
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

std::ostream& operator<<(std::ostream& os, const http_header::fields_t& httpFields) {
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
    os << requestHeader.method << " ";
    auto path_itr = requestHeader.fields.find(rohit::http_header::FIELD::Path);
    if (path_itr != requestHeader.fields.end()) {
        os << path_itr->second << " ";
    }
    return os << requestHeader.version << "\n"
              << requestHeader.fields << "\n";
}

std::ostream& operator<<(std::ostream& os, const http_header::CODE responseCODE) {
    return os << static_cast<int>(responseCODE) << " " << http_header::get_code_string(responseCODE);
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
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_md5();
    EVP_DigestInit_ex2(context, md, NULL);
    EVP_DigestUpdate(context, body.c_str(), body.length());

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    EVP_DigestFinal_ex(context, hash, &hash_len);
    EVP_MD_CTX_free(context);

    std::ostringstream md5Stream;
    md5Stream << std::hex << std::setfill('0');

    for(unsigned int index = 0; index < hash_len; ++index) md5Stream << std::setw(2) << static_cast<long long>(hash[index]);

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