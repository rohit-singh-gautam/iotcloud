////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/log.hh>
#include <iot/core/guid.hh>
#include <iot/core/error.hh>
#include <iot/core/math.hh>
#include <iot/states/states.hh>
#include <iot/net/socket.hh>
#include <sys/epoll.h>
#include <cstring>
#include <iostream>

namespace rohit {

logger<true> glog;
template<> size_t logger<true>::logger_count = 0;
template<> size_t logger<false>::logger_count = 0;
template<> logger<true> *logger<true>::logger_array[logger<true>::max_logger] = {};
template<> logger<false> *logger<false>::logger_array[logger<false>::max_logger] = {};


logreader::logreader(const std::string &filename) {
    // This must be read/write as same class can be used to read
    file_descriptor = open(filename.c_str(), O_RDONLY);
    if ( file_descriptor < 0 ) {
        std::cerr << "Failed to open file " << filename << ", error " << errno << "\n";
        throw exception_t(err_t::LOG_FILE_OPEN_FAILURE);
    }
}

constexpr void writeLogsText(const char * const source, size_t source_size, char *text, size_t &index) {
    std::copy(source, source + source_size, text + index);
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
    auto count =  to_string<T, radix, number_case, false>(value, pStr);
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
    auto count =  to_string<number_case, false>(value, pStr);
    pStr += count;
}

template <number_case number_case = number_case::lower>
void ipv6_addr_t_to_string_helper(char *&pStr, const uint8_t *&data_args) {
    const ipv6_addr_t &value = *(ipv6_addr_t *)data_args;
    data_args += sizeof(ipv6_addr_t);
    auto count =  to_string<number_case, false>(value, pStr);
    pStr += count;
}

void ipv6_port_t_to_string_helper(char *&pStr, const uint8_t *&data_args) {
    const ipv6_port_t &value = *(ipv6_port_t *)data_args;
    data_args += sizeof(ipv6_port_t);
    uint16_t port = value;
    auto count =  to_string<uint16_t, 10, number_case::lower, false>(port, pStr);
   
    pStr += count;
}

void errno_to_string(char *&pStr, const uint8_t *&data_args) {
    const int32_t errnum = *(int32_t *)data_args;
    data_args += sizeof(int32_t);
    const char *errstr = strerror(errnum);

    *pStr++ = '(';
    auto count =  to_string<int32_t, 10, number_case::lower, false>(errnum, pStr);
    pStr += count;
    *pStr++ = ',';

    const size_t len = strlen(errstr);
    std::copy(errstr, errstr + len, pStr);
    pStr += len;
    *pStr++ = ')';
}

void errno_t_to_string_helper(char *&pStr, const uint8_t *&data_args) {
    const err_t &value = *reinterpret_cast<const err_t *>(data_args);
    data_args += sizeof(err_t);
    auto count =  to_string<false>(value, pStr);
   
    pStr += count;
}

template <number_case number_case = number_case::lower>
void guid_t_to_string_helper(char *&pStr, const uint8_t *&data_args) {
    const guid_t &value = *reinterpret_cast<const guid_t *>(data_args);
    data_args += sizeof(guid_t);
    auto count =  to_string<number_case, false>(value, pStr);
   
    pStr += count;
}

void state_t_to_string_helper(char *&pStr, const uint8_t *&data_args) {
    const state_t &value = *reinterpret_cast<const state_t *>(data_args);
    data_args += sizeof(state_t);
    auto count =  to_string<false>(value, pStr);
   
    pStr += count;
}

template<std::size_t N>
void to_string_helper(char *&pStr, const char (&disp_str)[N]) {
    // Skipping null
    std::copy(disp_str, disp_str + N - 1, pStr);
    pStr += N - 1;
}

constexpr bool evt_contains(const uint32_t event, const uint32_t check_event) {
    return ((event & check_event) == check_event);
}

void epoll_event_to_string_helper(char *&pStr, const uint8_t *&data_args) {
    const uint32_t event = *(uint32_t *)data_args;
    data_args += sizeof(uint32_t);

    *pStr++ = '(';
    auto count =  to_string<int32_t, 10, number_case::lower, false>(event, pStr);
    pStr += count;
    *pStr++ = ',';

    if (evt_contains(event, EPOLLIN)) to_string_helper(pStr, "EPOLLIN");
    if (evt_contains(event, EPOLLPRI)) to_string_helper(pStr, "EPOLLPRI");
    if (evt_contains(event, EPOLLOUT)) to_string_helper(pStr, "EPOLLOUT");
    if (evt_contains(event, EPOLLRDNORM)) to_string_helper(pStr, "EPOLLRDNORM");
    if (evt_contains(event, EPOLLRDBAND)) to_string_helper(pStr, "EPOLLRDBAND");
    if (evt_contains(event, EPOLLWRNORM)) to_string_helper(pStr, "EPOLLWRNORM");
    if (evt_contains(event, EPOLLWRBAND)) to_string_helper(pStr, "EPOLLWRBAND");
    if (evt_contains(event, EPOLLMSG)) to_string_helper(pStr, "EPOLLMSG");
    if (evt_contains(event, EPOLLERR)) to_string_helper(pStr, "EPOLLERR");
    if (evt_contains(event, EPOLLHUP)) to_string_helper(pStr, "EPOLLHUP");
    if (evt_contains(event, EPOLLRDHUP)) to_string_helper(pStr, "EPOLLRDHUP");
    if (evt_contains(event, EPOLLEXCLUSIVE)) to_string_helper(pStr, "EPOLLEXCLUSIVE");
    if (evt_contains(event, EPOLLWAKEUP)) to_string_helper(pStr, "EPOLLWAKEUP");
    if (evt_contains(event, EPOLLONESHOT)) to_string_helper(pStr, "EPOLLONESHOT");
    if (evt_contains(event, EPOLLET)) to_string_helper(pStr, "EPOLLET"); 
    *pStr++ = ')';   
}


static inline void flush_all_logger(const int filedescriptor) {
    logger<true>::flushall(filedescriptor);
    logger<false>::flushall(filedescriptor);
}

pthread_t log_thread;
int log_filedescriptor = 0;
bool log_thread_running = false;

static void *log_thread_function(void *) {
    constexpr auto wait_time = std::chrono::milliseconds(config::log_thread_wait_in_millis);
    log_thread_running = true;
    while(log_thread_running) {
        std::this_thread::sleep_for(wait_time);
        flush_all_logger(log_filedescriptor);
    }

    flush_all_logger(log_filedescriptor);

    return nullptr;
}

void init_log_thread(const char *filename) {
    log_filedescriptor = open(filename, O_RDWR | O_APPEND | O_CREAT, O_SYNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if ( log_filedescriptor < 0 ) {
        std::cerr << "Failed to open file " << filename << ", error " << errno << ", " << strerror(errno) << std::endl;
    }

    auto ret = pthread_create(&log_thread, NULL, &log_thread_function, nullptr);
    if (ret != 0) {
        std::cerr << "Failed to create thread, error " << ret << ", " << strerror(ret) << std::endl;
    }
}


void destroy_log_thread() {
    log_thread_running = false;
    auto ret = pthread_join(log_thread, nullptr);
    if (ret != 0) {
        std::cerr << "Failed to join log thread, error " << errno << ", " << strerror(errno) << std::endl;
    }
    close(log_filedescriptor);
    log_filedescriptor = 0;
}


template <size_t N>
constexpr void write_string(char *&pStr, const char (&message)[N]) {
    constexpr size_t n = N-1;
    std::copy(message, message + n, pStr);
    pStr += n;
}

void createLogsString(logger_logs_entry_read &logEntry, char *text) {
    const uint8_t *data_args = logEntry.arguments;

    size_t index = 0;

    switch(logEntry.id) {
        default:
            assert(true);
#define LOGGER_ENTRY(x, y, m, z) case log_t::x: writeLogsText(#y, sizeof(#y) - 1, text, index); break;
            LOGGER_LOG_LIST
#undef LOGGER_ENTRY
    }

    *(text + index++) = ':';

    const char *id_str = get_log_id_string(logEntry.id);
    writeLogsText(id_str, text, index);
    const char *desc_str = get_log_description(logEntry.id);

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
                case 'e': errno_to_string(pStr, data_args); break;
                case 'E': errno_t_to_string_helper(pStr, data_args); break;
                case 'g': guid_t_to_string_helper(pStr, data_args); break;
                case 'G': guid_t_to_string_helper<number_case::upper>(pStr, data_args); break;
                case 'v': epoll_event_to_string_helper(pStr, data_args); break;
                case 's': state_t_to_string_helper(pStr, data_args); break;
                default: write_string(pStr, "Unknown message, client may required to be upgraded"); break;
            } // switch (c)
            state = formatstring_state::COPY;
            break; // case formatstring_state::MODIFIER_CUSTOM:
        } // switch (state)
    } // while(*desc_str)

    *pStr++ = '\0';
}

ssize_t log_read_helper(int fd, void *buf, size_t n) {
    if (n == 0) return 0;
    ssize_t read_size = read(fd, (void *)buf, n);
    while(read_size == 0) {
        read_size = read(fd, (void *)buf, n);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (read_size != n) {
        std::cerr << "Read failure " << errno << std::endl;
        throw exception_t(err_t::LOG_READ_FAILURE);
    }

    return read_size;
}


const std::string logreader::readnext() {
    // First read header
    logger_logs_entry_common &log_common = *(logger_logs_entry_common *)data_args;
    logger_logs_entry_end_of_cluster &log_end_of_cluster = *(logger_logs_entry_end_of_cluster *)data_args;
    logger_logs_entry_read &log_read = *(logger_logs_entry_read *)data_args;
    ssize_t total_read = 0;

    auto read_size = log_read_helper(file_descriptor, (void *)data_args, sizeof(logger_logs_entry_common));

    if (log_common.id == log_t::END_OF_CLUSTER) {
        // Reading end of cluster length
        read_size = log_read_helper(
            file_descriptor,
            (void *)(data_args + read_size),
            sizeof(logger_logs_entry_end_of_cluster) - sizeof(logger_logs_entry_common));

        // Skipping end of cluster length
        read_size = log_read_helper(
            file_descriptor,
            data_args + sizeof(logger_logs_entry_end_of_cluster),
            log_end_of_cluster.length - sizeof(logger_logs_entry_end_of_cluster));

        read_size = log_read_helper(file_descriptor, (void *)data_args, sizeof(logger_logs_entry_common));
    }

    total_read += read_size;

    size_t data_args_size = get_log_length(log_common.id);
    read_size = log_read_helper(file_descriptor, (void *)(data_args + total_read), data_args_size);

    createLogsString(log_read, text);

    return std::string(text);
}

} // namespace rohit