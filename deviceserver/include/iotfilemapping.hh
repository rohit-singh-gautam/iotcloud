////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/error.hh>
#include <iot/core/memory_helper.hh>
#include <iot/core/math.hh>
#include <time.h>
#include <sys/stat.h>
#include <string>
#include <memory>
#include <unordered_map>

namespace rohit::http {
struct file_map_param {
    const ipv6_port_t port;
    const std::string file; // Name with path
    file_map_param(const ipv6_port_t port, const std::string &file) : port(port), file(file) {}

    bool operator==(const file_map_param &rhs) const {
        return rhs.port == port && rhs.file == file;
    }
};
} // namespace rohit::http

namespace std {
template<>
struct hash<rohit::http::file_map_param>
{
    size_t
    operator()(const rohit::http::file_map_param &val) const noexcept
    {
        return std::_Hash_impl::__hash_combine(val.port, std::hash<std::string>{}(val.file));
    }
};
} // namespace std

namespace rohit::http {

struct file_info {
    const char *text;
    const size_t text_size;
    const char *type; //Content-Type
    const size_t type_size;
    const char *etags;
    static constexpr size_t etags_size = to_string64_hash<uint64_t, false>();

    constexpr file_info(
                const char *text,
                const size_t text_size,
                const char *type,
                const size_t type_size,
                const char *etags)
        : text(text), text_size(text_size), type(type), type_size(type_size), etags(etags) { }

    ~file_info() {
        delete[] text;
        delete[] type;
        delete[] etags;
    }
};

class filemap {
    // This is designed to be permanent
    // It is expected to restart server once file is changed
public:

    std::unordered_map<file_map_param, std::shared_ptr<file_info>> cache;
    std::unordered_map<std::string, std::string> content_type_map; // Extension, file

    filemap();

    err_t add_folder(const ipv6_port_t port, const std::string &webfolder, const std::string &folder = "/");

    // Mapping will be done only if destination is present
    err_t additional_mapping(const file_map_param &source, const file_map_param &dest);

    void add_file(const ipv6_port_t port, const std::string &webfolder, const std::string &filepath);
};

extern filemap webfilemap;

} // namespace rohit::http