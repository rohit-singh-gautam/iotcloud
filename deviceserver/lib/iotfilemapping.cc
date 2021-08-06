////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/error.hh>
#include <iotfilemapping.hh>
#include <dirent.h>
#include <unistd.h>

namespace rohit::http {

filemap webfilemap;

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

void filemap::add_file(const ipv6_port_t port, const std::string &webfolder, const std::string &relativepath) {
    file_map_param mapparam = {port, relativepath};
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

    char *etag_buffer = new char[to_string64_hash<uint64_t, false>()];
    to_string64_hash(etag, etag_buffer);

    std::shared_ptr<file_info> file_details = std::make_shared<file_info>(buffer, size, etag_buffer);

    cache.insert(std::make_pair(mapparam, std::shared_ptr<file_info>(file_details)));
}

err_t filemap::add_folder(const ipv6_port_t port, const std::string &webfolder, const std::string &folder) {
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
            add_folder(port, webfolder, folder + std::string(child->d_name) + "/");
            break;
        }
        case DT_REG: {
            add_file(port, webfolder, folder + std::string(child->d_name));
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

err_t filemap::additional_mapping(const file_map_param &source, const file_map_param &dest) {
    auto value = cache.find(source);
    if (value == cache.end()) {
        return err_t::HTTP_FILEMAP_NOT_FOUND;
    }
    cache.insert(std::make_pair(dest, value->second));
    return err_t::SUCCESS;
}


} // namespace rohit::http