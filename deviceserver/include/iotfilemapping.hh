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

    inline file_info(
                const char *text,
                const size_t text_size,
                const char *type,
                const size_t type_size,
                const char *etags)
        : text(text), text_size(text_size), type(type), type_size(type_size), etags(etags) { }

    inline ~file_info() {
        delete[] text;
        delete[] type;
        delete[] etags;
    }
};

class filemap {
private:
    std::unordered_map<std::string, std::string> folder_mappings;
    std::unordered_map<std::string, std::vector<std::string>> folder_reverse_mappings;
    std::unordered_map<std::string, std::string> content_type_map; // Extension, file
    const std::string webfolder;

public:
    std::unordered_map<std::string, std::shared_ptr<file_info>> cache;

    filemap(const std::string &webfolder)
        : folder_mappings(), folder_reverse_mappings(), content_type_map(), webfolder(webfolder), cache() {}

    void update_folder();
    void flush_cache();

    void insert_folder_mapping(const std::string &source, const std::string &destination);
    void update_folder_mapping();

    void insert_content_type(const std::string &extension, const std::string &content_type);

    void add_file(const std::string &filepath);
    err_t add_folder(const std::string &folder);

    void modify_file(const std::string &filepath);

    void remove_file(const std::string &filepath);
    void remove_folder(const std::string &folder);
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

    void add_folder(const ipv6_port_t port, const std::string &webfolder);
    void update_folder();
    void flush_cache();
    err_t update_folder(const std::string &webfolder);
    err_t flush_cache(const std::string &webfolder);

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