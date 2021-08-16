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

struct file_info {
    const char *text;
    const size_t text_size;
    const char *type; //Content-Type
    const size_t type_size;
    const char *etags;
    static constexpr size_t etags_size = to_string64_hash<uint64_t>();

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
    err_t add_folder(const std::string &webfolder, const std::string &folder);

public:

    std::unordered_map<std::string, std::shared_ptr<file_info>> cache;
    std::unordered_map<std::string, std::string> content_type_map; // Extension, file
    std::unordered_map<std::string, std::string> folder_mappings;

    filemap() : cache(), content_type_map(), folder_mappings() {}

    inline err_t add_folder(const std::string &webfolder) {
        return add_folder(webfolder, "/");
    }

    // Mapping will be done only if destination is present
    err_t additional_mapping(const std::string &source, const std::string &dest);

    void add_file(const std::string &webfolder, const std::string &filepath);
};

class webmaps {
    // This is many:1 mapping
    std::unordered_map<ipv6_port_t, filemap *> webportmaps;

    // This is 1:1 mapping
    std::unordered_map<std::string, filemap *> webfoldermaps;
public:
    webmaps() : webportmaps(), webfoldermaps()  {}
    ~webmaps() {
        for(auto &webfoldermap: webfoldermaps) {
            delete webfoldermap.second;
        }
    }

    err_t add_folder(const ipv6_port_t port, const std::string &webfolder);
    err_t update_folder();

    err_t add_folder_mapping(
                const std::string &webfolder,
                std::string source,
                std::string destination);

    err_t add_content_type_mapping(
                const std::string &webfolder,
                const std::string &extension,
                const std::string &content_type);

    filemap * getfilemap(ipv6_port_t port) {
        auto file_mapping = webportmaps.find(port);
        if (file_mapping == webportmaps.end()) {
            throw exception_t(err_t::HTTP_FILEMAP_NOT_FOUND);
        }

        return file_mapping->second;
    }
};

extern webmaps webfilemap;

} // namespace rohit::http