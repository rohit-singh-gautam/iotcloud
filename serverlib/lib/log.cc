#include <iot/log.hh>
#include <iot/core/error.hh>
#include <iot/math.hh>
#include <iot/socket.hh>
#include <cstring>
#include <iostream>

namespace rohit {

class log_cluster_entry {
public:
    static constexpr size_t max_cluster_size = 8192;
    bool used = false;
    size_t start = 0;
    size_t index = 0;
    uint8_t buffer[max_cluster_size];

    constexpr void init() {
        start = index = 0;
    }
};

class log_buffer {
public:

    static constexpr size_t max_cluster_count = 2;
    size_t current_cluster = 0;
    log_cluster_entry cluster_list[max_cluster_count];

    constexpr log_cluster_entry *get_current_cluster();

    constexpr void init() {
        current_cluster = 0;
        for(size_t count = 0; count < max_cluster_count; ++count) {
            cluster_list[count].init();
        }
    }

    // For timebeing no check will be performed
    constexpr void next_cluster() {
        ++current_cluster;
        if (current_cluster == max_cluster_count) current_cluster = 0;
        log_cluster_entry &log_cluster = *get_current_cluster();
        log_cluster.index = 0;
        log_cluster.start = 0;
    }
};

constexpr log_cluster_entry *log_buffer::get_current_cluster() {
    return cluster_list + current_cluster;
}

log_buffer log_buf;

int logger::file_descriptor = 0;

void logger::write(const void *data, const size_t length) {
    log_cluster_entry *log_cluster = log_buf.get_current_cluster();

    if (log_cluster->index + length + sizeof(logger_logs_entry_end_of_cluster) > log_cluster_entry::max_cluster_size) {
        auto ret = ::write(
            file_descriptor,
            log_cluster->buffer + log_cluster->start,
            log_cluster->index - log_cluster->start);
        if (ret < 0) {
            // Log to console
            std::cerr << "Failed to write log with error: " << errno << "\n";
        }

        logger_logs_entry_end_of_cluster endofcluster(log_cluster_entry::max_cluster_size - log_cluster->index);
        memcpy(log_cluster->buffer, (void *)&endofcluster, sizeof(logger_logs_entry_end_of_cluster));
        ret = ::write(file_descriptor, log_cluster->buffer, endofcluster.length);

        log_buf.next_cluster();
        log_cluster = log_buf.get_current_cluster();
    }

    memcpy(log_cluster->buffer + log_cluster->index, data, length);
    log_cluster->index += length;
}

void logger::init(const std::string &filename) {
    log_buf.init();
    // This must be read/write as same class can be used to read
    file_descriptor = open(filename.c_str(), O_RDWR | O_APPEND | O_CREAT, O_SYNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if ( file_descriptor < 0 ) {
        std::cerr << "Failed to open file " << filename << ", error " << errno << "\n";
    } else {
        log_cluster_entry *log_cluster = log_buf.get_current_cluster();
         auto index = lseek(file_descriptor, 0, SEEK_CUR);
         log_cluster->start = log_cluster->index = index;

    }
}

void logger::flush() {
    log_cluster_entry *log_cluster = log_buf.get_current_cluster();
    if (log_cluster->start == log_cluster->index) return;
    
    auto ret = ::write(
        file_descriptor,
        log_cluster->buffer + log_cluster->start,
        log_cluster->index - log_cluster->start);
    if (ret < 0) {
        // Log to console
        std::cerr << "Failed to write log with error: " << errno << "\n";
    } else {
        log_cluster->start = log_cluster->index;
    }
}

logreader::logreader(const std::string &filename) {
    // This must be read/write as same class can be used to read
    file_descriptor = open(filename.c_str(), O_RDONLY);
    if ( file_descriptor < 0 ) {
        std::cerr << "Failed to open file " << filename << ", error " << errno << "\n";
        throw exception_t(exception_t::LOG_FILE_OPEN_FAILURE);
    }
}

constexpr void writeLogsText(const char * const source, size_t source_size, char *text, size_t &index) {
    memcpy(text + index, source, source_size);
    index += source_size;
}

constexpr void writeLogsText(const char * source, char *text, size_t &index) {
   while(*source) {
       *(text + index++) = *source++;
   }
}

template <typename T, T radix = 10, number_case number_case = number_case::lower>
void integerToStringHelper(char *&pStr, const uint8_t *&data_args) {
    T value = *(T *)data_args;
    data_args += sizeof(T);
    auto count =  to_string<T, radix, number_case, false>(pStr, value);
    pStr += count;
}

template <typename T>
void floatToStringHelper(char *&pStr, const uint8_t *&data_args) {
    T value = *(T *)data_args;
    data_args += sizeof(T);
    auto count =  floatToString<T, false>(pStr, value);
    pStr += count;
}

template <number_case number_case = number_case::lower>
void ipv6_socket_addr_t_to_string_helper(char *&pStr, const uint8_t *&data_args) {
    const ipv6_socket_addr_t &value = *(ipv6_socket_addr_t *)data_args;
    data_args += sizeof(ipv6_socket_addr_t);
    auto count =  to_string<number_case, false>(pStr, value);
    pStr += count;
}

template <number_case number_case = number_case::lower>
void ipv6_addr_t_to_string_helper(char *&pStr, const uint8_t *&data_args) {
    const ipv6_addr_t &value = *(ipv6_addr_t *)data_args;
    data_args += sizeof(ipv6_addr_t);
    auto count =  to_string<number_case, false>(pStr, value);
    pStr += count;
}

void ipv6_port_t_to_string_helper(char *&pStr, const uint8_t *&data_args) {
    const ipv6_port_t &value = *(ipv6_port_t *)data_args;
    data_args += sizeof(ipv6_port_t);
    uint16_t port = value;
    auto count =  to_string<uint16_t, 10, number_case::lower, false>(pStr, port);
    pStr += count;
}

void createLogsString(logger_logs_entry_read &logEntry, char *text) {
    const uint8_t *data_args = logEntry.arguments;
    if (logEntry.log_type != logger_type::LOGS) {
        throw exception_t(exception_t::LOG_UNSUPPORTED_TYPE_FAILURE);
    }

    size_t index = 0;

    switch(logEntry.level) {
#define LOGGER_LEVEL_ENTRY(x) case logger_level::x: writeLogsText(#x, sizeof(#x) - 1, text, index); break;
    LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY
    }

    *(text + index++) = ':';

    const char *id_str = logger::id_string(logEntry.id);
    writeLogsText(id_str, text, index);
    const char *desc_str = logger::id_description(logEntry.id);

    char *pStr = text + index;
    *pStr++ = ':';

    formatstring_state state = formatstring_state::COPY;
    formatstring_type_length lenght_specifier = formatstring_type_length::NONE;
    while(*desc_str) {
        char c = *desc_str++;
        switch (state)
        {
        case formatstring_state::COPY:
            switch (c)
            {
            case '%':
                state = formatstring_state::MODIFIER;
                lenght_specifier = formatstring_type_length::NONE;
                break;
            
            default:
                *pStr++ = c;
                break;
            }
            break;

        case formatstring_state::MODIFIER:
            switch (c)
            {
            case '%':
                state = formatstring_state::COPY;
                *pStr++ = c;
                break;
            
            case 'd':
            case 'i': {
                switch(lenght_specifier) {
                case formatstring_type_length::h: integerToStringHelper<int16_t>(pStr, data_args); break;
                case formatstring_type_length::hh: integerToStringHelper<int8_t>(pStr, data_args); break;
                case formatstring_type_length::NONE: integerToStringHelper<int32_t>(pStr, data_args); break;
                case formatstring_type_length::l:
                case formatstring_type_length::ll: integerToStringHelper<int64_t>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'u': {
                switch(lenght_specifier) {
                case formatstring_type_length::NONE: integerToStringHelper<uint32_t>(pStr, data_args); break;
                case formatstring_type_length::h: integerToStringHelper<uint16_t>(pStr, data_args); break;
                case formatstring_type_length::hh: integerToStringHelper<uint8_t>(pStr, data_args); break;
                case formatstring_type_length::l:
                case formatstring_type_length::ll: integerToStringHelper<uint64_t>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'o': {
                switch(lenght_specifier) {
                case formatstring_type_length::NONE: integerToStringHelper<uint32_t, 8>(pStr, data_args); break;
                case formatstring_type_length::h: integerToStringHelper<uint16_t, 8>(pStr, data_args); break;
                case formatstring_type_length::hh: integerToStringHelper<uint8_t, 8>(pStr, data_args); break;
                case formatstring_type_length::l:
                case formatstring_type_length::ll: integerToStringHelper<uint64_t, 8>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'x': {
                switch(lenght_specifier) {
                case formatstring_type_length::NONE: integerToStringHelper<uint32_t, 16>(pStr, data_args); break;
                case formatstring_type_length::h: integerToStringHelper<uint16_t, 16>(pStr, data_args); break;
                case formatstring_type_length::hh: integerToStringHelper<uint8_t, 16>(pStr, data_args); break;
                case formatstring_type_length::l:
                case formatstring_type_length::ll: integerToStringHelper<uint64_t, 16>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'X': {
                switch(lenght_specifier) {
                case formatstring_type_length::NONE: integerToStringHelper<uint32_t, 16, number_case::upper>(pStr, data_args); break;
                case formatstring_type_length::h: integerToStringHelper<uint16_t, 16, number_case::upper>(pStr, data_args); break;
                case formatstring_type_length::hh: integerToStringHelper<uint8_t, 16, number_case::upper>(pStr, data_args); break;
                case formatstring_type_length::l:
                case formatstring_type_length::ll: integerToStringHelper<uint64_t, 16, number_case::upper>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'f': {
                switch(lenght_specifier) {
                default:
                case formatstring_type_length::NONE: floatToStringHelper<float>(pStr, data_args); break;
                case formatstring_type_length::l: floatToStringHelper<double>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'c':
                *pStr++ = *data_args++;
                break;
            
            case 'v':
                state = formatstring_state::MODIFIER_CUSTOM;
                break;

            // Specifiers
            case 'h':
                if (lenght_specifier == formatstring_type_length::h)
                    lenght_specifier = formatstring_type_length::hh;
                else lenght_specifier = formatstring_type_length::h;
                break;
            
            case 'l':
                if (lenght_specifier == formatstring_type_length::l)
                    lenght_specifier = formatstring_type_length::ll;
                else lenght_specifier = formatstring_type_length::l;
                break;
            
            default: // Junk character we just ignoring it
                state = formatstring_state::COPY;
                break;
            } // switch (c)
            break; // case formatstring_state::MODIFIER:
        
        case formatstring_state::MODIFIER_CUSTOM:
            switch (c) {
                case 'n': ipv6_socket_addr_t_to_string_helper(pStr, data_args); break;
                case 'N': ipv6_socket_addr_t_to_string_helper<number_case::upper>(pStr, data_args); break;
                case 'i': ipv6_addr_t_to_string_helper(pStr, data_args); break;
                case 'I': ipv6_addr_t_to_string_helper<number_case::upper>(pStr, data_args); break;
                case 'p': ipv6_port_t_to_string_helper(pStr, data_args); break;
                    break;
            } // switch (c)
            state = formatstring_state::COPY;
            break; // case formatstring_state::MODIFIER_CUSTOM:
        } // switch (state)
    } // while(*desc_str)

    *pStr++ = '\0';
}


const std::string logreader::readnext() {
    // First read header
    logger_logs_entry_header &log_header = *(logger_logs_entry_header *)data_args;
    logger_logs_entry_end_of_cluster &log_end_of_cluster = *(logger_logs_entry_end_of_cluster *)data_args;
    logger_logs_entry_common &log_common = *(logger_logs_entry_common *)data_args;
    logger_logs_entry_read &log_read = *(logger_logs_entry_read *)data_args;
    ssize_t total_read = 0;

    ssize_t read_size = read(file_descriptor, (void *)data_args, sizeof(logger_logs_entry_header));
    if (read_size != sizeof(logger_logs_entry_header)) {
        std::cerr << "Read failure " << errno << std::endl;
        throw exception_t(exception_t::LOG_READ_FAILURE);
    }

    if (log_header.log_type == logger_type::END_OF_CLUSTER) {
        // Reading end of cluster length
        read_size = read(
            file_descriptor,
            (void *)(data_args + read_size),
            sizeof(logger_logs_entry_end_of_cluster) - sizeof(logger_logs_entry_header));
        if (read_size != sizeof(logger_logs_entry_end_of_cluster) - sizeof(logger_logs_entry_header)) {
            std::cerr << "Read failure " << errno << std::endl;
            throw exception_t(exception_t::LOG_READ_FAILURE);
        }

        // Skipping end of cluster length
        read_size = read(
            file_descriptor,
            data_args + sizeof(logger_logs_entry_end_of_cluster),
            log_end_of_cluster.length - sizeof(logger_logs_entry_end_of_cluster));
        if ((size_t)read_size != log_end_of_cluster.length - sizeof(logger_logs_entry_end_of_cluster)) {
            std::cerr << "Read failure " << errno << std::endl;
            throw exception_t(exception_t::LOG_READ_FAILURE);
        }

        read_size = read(file_descriptor, (void *)data_args, sizeof(logger_logs_entry_header));
        if (read_size != sizeof(logger_logs_entry_header)) {
            std::cerr << "Read failure " << errno << std::endl;
            throw exception_t(exception_t::LOG_READ_FAILURE);
        }

        if (log_header.log_type != logger_type::LOGS) {
            std::cerr << "Read failure wronge logger type" << errno << std::endl;
            throw exception_t(exception_t::LOG_READ_FAILURE);
        }
    }

    total_read += read_size;

    read_size = read(file_descriptor, (void *)(data_args + total_read), sizeof(logger_logs_entry_common) - total_read);

    if ((size_t)read_size != sizeof(logger_logs_entry_common) - total_read) {
        std::cerr << "Read failure " << errno << std::endl;
        throw exception_t(exception_t::LOG_READ_FAILURE);
    }

    total_read += read_size;

    size_t data_args_size = get_log_length(log_common.id);
    read_size = read(file_descriptor, (void *)(data_args + total_read), data_args_size);

    if (read_size != (ssize_t)data_args_size) {
        throw exception_t(exception_t::LOG_READ_FAILURE);
    }

    createLogsString(log_read, text);

    return std::string(text);
}

} // namespace rohit