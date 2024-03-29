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
    mem_new<const char> content;
    mem_new<const char> content_type; //Content-Type
    const char *etags;
    static constexpr size_t etags_size = to_string64_hash<uint64_t>();

    inline file_info(
                mem_new<const char> content,
                mem_new<const char> content_type,
                const char *etags)
        : content(content), content_type(content_type), etags(etags) { }

    inline file_info(
                const char *content,
                size_t content_size,
                const char *content_type,
                size_t content_type_size,
                const char *etags)
        : content(content, content_size), content_type(content_type, content_type_size), etags(etags) { }

    inline ~file_info() {
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