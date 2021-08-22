////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/log.hh>
#include <iot/core/error.hh>
#include <iotfilemapping.hh>
#include <dirent.h>
#include <unistd.h>

namespace rohit::http {

webmaps webfilemap;

// Get etags from FD
// Etags 
inline uint64_t get_etags(int fd) {
    struct stat statbuf;
    auto ret = fstat(fd, &statbuf);
    if (ret == -1) throw exception_t(err_t::BAD_FILE_DESCRIPTOR);

    uint64_t nanos = (uint64_t)statbuf.st_mtim.tv_sec * 1000000000 + (uint64_t)statbuf.st_mtim.tv_nsec;
    nanos = nanos & (~0xfffff);
    nanos += statbuf.st_size;

    return nanos;
}

void filemap::add_file(const std::string &relativepath) {
    size_t period_pos = relativepath.rfind('.');
    if (period_pos == std::string::npos) {
        glog.log<log_t::WEB_SERVER_NO_EXTENSION>();
        return;
    }
    const std::string extension = relativepath.substr(period_pos + 1);

    const auto content_type_iter = content_type_map.find(extension);
    if (content_type_iter == content_type_map.end()) {
        glog.log<log_t::WEB_SERVER_UNSUPPORTED_EXTENSION>();
        return;
    }

    const auto &content_type_str = content_type_iter->second;

    auto filepath = webfolder + relativepath;

    int fd = open(filepath.c_str(), O_RDONLY);
    if ( fd == -1 ) {
        perror("Unable to open file");
        return;
    }

    struct stat bufstat;
    fstat(fd, &bufstat);

    int size = bufstat.st_size;
    char *buffer = new char[size];
    auto read_size = read(fd, buffer, size);

    uint64_t etag = get_etags(fd);
    close(fd);

    char *etag_buffer = new char[to_string64_hash<uint64_t>()];
    to_string64_hash(etag, etag_buffer);

    char *content_type_buffer = new char[content_type_str.length() + 1];
    content_type_buffer[content_type_str.length()] = '\0'; // null termination
    std::copy(content_type_str.begin(), content_type_str.end(), content_type_buffer);

    std::shared_ptr<file_info> file_details = std::make_shared<file_info>(
        buffer, size,
        content_type_buffer, content_type_str.length() + 1,
        etag_buffer);

    cache.insert(std::make_pair(relativepath, std::shared_ptr<file_info>(file_details)));
}

err_t filemap::add_folder(const std::string &folder) {
    const auto constfolder = webfolder + folder;
    DIR * dir = opendir(constfolder.c_str());

    if (!dir) {
        // TODO: return specific error
        return err_t::GENERAL_FAILURE;
    }

    while(1) {
        dirent *child = readdir(dir);
        if (!child) break;

        if ( !strcmp(child->d_name, ".") || !strcmp(child->d_name, "..") ) {
            // Ignore current and parent directory
            continue;
        }
        
        switch(child->d_type) {
        case DT_DIR: {
            // ignoring child error
            add_folder(folder + std::string(child->d_name) + "/");
            break;
        }
        case DT_REG: {
            add_file(folder + std::string(child->d_name));
            break;
        }
        
        default:
            // Other file types we will ignore
            break;
        }
    }


    if (closedir(dir)) {
        // TODO: return errno specific error
        return err_t::GENERAL_FAILURE;
    }

    return err_t::SUCCESS;
}

void filemap::update_folder() {
    add_folder("/");
    update_folder_mapping();
}

void filemap::flush_cache() {
    cache.clear();
}

void filemap::insert_folder_mapping(const std::string &source, const std::string &destination) {
    folder_mappings.insert(std::make_pair(source, destination));
    
    auto dest_itr = folder_reverse_mappings.find(destination);
    if (dest_itr == folder_reverse_mappings.end()) {
        std::vector<std::string> source_list;
        source_list.push_back(source);
        folder_reverse_mappings.insert(std::make_pair(destination, source_list));
    } else {
        dest_itr->second.push_back(source);
    }
}

void filemap::update_folder_mapping() {
    for(auto &map_pair: folder_mappings) {
        const std::string &source = map_pair.first;
        const std::string &dest = map_pair.second;
        auto value = cache.find(dest);
        if (value != cache.end()) {
            cache.insert(std::make_pair(source, value->second));
        }
    }
}

void filemap::insert_content_type(const std::string &extension, const std::string &content_type) {
    content_type_map.insert(std::make_pair(extension, content_type));
}

void webmaps::add_folder(const ipv6_port_t port, const std::string &webfolder) {
    filemap *maps;
    auto folder_itr = webfoldermaps.find(webfolder);
    if (folder_itr == webfoldermaps.end()) {
        maps = new filemap(webfolder);
        webfoldermaps.insert(std::make_pair(webfolder, maps));
    } else {
        maps = folder_itr->second;
    }

    auto port_itr = webportmaps.find(port);
    if (port_itr == webportmaps.end()) {
        webportmaps.insert(std::make_pair(port, maps));
    } else {
        webportmaps[port] = maps;
    }
}

void webmaps::update_folder() {
    for(auto &folder_pair: webfoldermaps) {
        folder_pair.second->update_folder();
        folder_pair.second->update_folder_mapping();
    }
}

void webmaps::flush_cache() {
    for(auto &folder_pair: webfoldermaps) {
        folder_pair.second->flush_cache();
    }
}

err_t webmaps::update_folder(const std::string &webfolder) {
    auto filemap_itr = webfoldermaps.find(webfolder);
    if (filemap_itr == webfoldermaps.end()) {
        return err_t::HTTP_FILEMAP_NOT_FOUND;
    }

    auto pfilemap = filemap_itr->second;

    pfilemap->update_folder();
    pfilemap->update_folder_mapping();

    return err_t::SUCCESS;
}


err_t webmaps::flush_cache(const std::string &webfolder) {
    auto filemap_itr = webfoldermaps.find(webfolder);
    if (filemap_itr == webfoldermaps.end()) {
        return err_t::HTTP_FILEMAP_NOT_FOUND;
    }

    auto pfilemap = filemap_itr->second;

    pfilemap->flush_cache();

    return err_t::SUCCESS;
}

err_t webmaps::add_folder_mapping(
                const std::string &webfolder,
                std::string source,
                std::string destination) {
    if (webfolder == "*") {
        for(auto &folder_pair: webfoldermaps) {
            folder_pair.second->insert_folder_mapping(source, destination);
        }
    } else {
        auto folder_itr = webfoldermaps.find(webfolder);
        if (folder_itr == webfoldermaps.end()) {
            return err_t::HTTP_FILEMAP_NOT_FOUND;
        }

        folder_itr->second->insert_folder_mapping(source, destination);
    }

    return err_t::SUCCESS;
}

err_t webmaps::add_content_type_mapping(
            const std::string &webfolder,
            const std::string &extension,
            const std::string &content_type) {
    if (webfolder == "*") {
        for(auto &folder_pair: webfoldermaps) {
            folder_pair.second->insert_content_type(extension, content_type);
        }
    } else {
        auto folder_itr = webfoldermaps.find(webfolder);
        if (folder_itr == webfoldermaps.end()) {
            return err_t::HTTP_FILEMAP_NOT_FOUND;
        }
        folder_itr->second->insert_content_type(extension, content_type);
    }
    return err_t::SUCCESS;
}

} // namespace rohit::http