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
#include <chrono>

namespace rohit {

thread_local logger _log;

logger_list::~logger_list() {
    logger_list::flush();
    enabled = false;
    close(fd);
}

void logger_list::add(logger *new_logger)
{
    if (enabled) {
        std::lock_guard guard {mutex};
        logger_store.insert(new_logger);
    }
}

void logger_list::remove(logger *new_logger)
{
    if (enabled) {
        std::lock_guard guard {mutex};
        logger_store.erase(new_logger);
    }
}

void logger_list::flush() {
    if (enabled) {
        std::lock_guard guard {mutex};
        std::ranges::for_each(logger_store, [this](auto &logger) { logger->flush(this->fd); });
    }
}

void logger_list::set_fd(const int fd) {
    this->fd = fd;
}

logger_list logger::all { };

active_module enabled_log_module;

logreader::logreader(const std::string &filename) : 
    file_descriptor(open(filename.c_str(), O_RDONLY)),
    text(),
    priqueue() {
    // This must be read/write as same class can be used to read
    if ( file_descriptor < 0 ) {
        std::cerr << "Failed to open file " << filename << ", error " << errno << "\n";
        throw exception_t(err_t::LOG_FILE_OPEN_FAILURE);
    }
}

constexpr void writeLogsText(const char * const source, size_t source_size, char *&pStr) {
    std::copy(source, source + source_size, pStr);
    pStr += source_size;
}

constexpr void writeLogsText(const char * source, char *&pStr) {
   while(*source) {
       *pStr++ = *source++;
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

    if (evt_contains(event, EPOLLIN)) to_string_helper(pStr, " EPOLLIN");
    if (evt_contains(event, EPOLLPRI)) to_string_helper(pStr, " EPOLLPRI");
    if (evt_contains(event, EPOLLOUT)) to_string_helper(pStr, " EPOLLOUT");
    if (evt_contains(event, EPOLLRDNORM)) to_string_helper(pStr, " EPOLLRDNORM");
    if (evt_contains(event, EPOLLRDBAND)) to_string_helper(pStr, " EPOLLRDBAND");
    if (evt_contains(event, EPOLLWRNORM)) to_string_helper(pStr, " EPOLLWRNORM");
    if (evt_contains(event, EPOLLWRBAND)) to_string_helper(pStr, " EPOLLWRBAND");
    if (evt_contains(event, EPOLLMSG)) to_string_helper(pStr, " EPOLLMSG");
    if (evt_contains(event, EPOLLERR)) to_string_helper(pStr, " EPOLLERR");
    if (evt_contains(event, EPOLLHUP)) to_string_helper(pStr, " EPOLLHUP");
    if (evt_contains(event, EPOLLRDHUP)) to_string_helper(pStr, " EPOLLRDHUP");
    if (evt_contains(event, EPOLLEXCLUSIVE)) to_string_helper(pStr, " EPOLLEXCLUSIVE");
    if (evt_contains(event, EPOLLWAKEUP)) to_string_helper(pStr, " EPOLLWAKEUP");
    if (evt_contains(event, EPOLLONESHOT)) to_string_helper(pStr, " EPOLLONESHOT");
    if (evt_contains(event, EPOLLET)) to_string_helper(pStr, " EPOLLET"); 
    *pStr++ = ')';   
}

void add_time_to_string_helper(
    char *&pStr,
    const int64_t nanotime)
{
    std::chrono::system_clock::duration dur(nanotime);
    std::chrono::system_clock::time_point currentTime(dur);

    auto transformed = nanotime / 1000000;
    auto nanos = nanotime % 1000000;

    auto millis = transformed % 1000;

    std::time_t tt;
    tt = std::chrono::system_clock::to_time_t ( currentTime );
    auto timeinfo = localtime (&tt);
    int count = strftime(pStr, 92, "%F %T", timeinfo);
    pStr += count;
    count = sprintf(pStr, ".%03ld.%06ld", millis, nanos);

    pStr += count;
}

pthread_t log_thread;
bool log_thread_running = false;

void segv_log_flush() {
    logger::all.flush();
}

static void *log_thread_function(void *) {
    constexpr auto wait_time = std::chrono::milliseconds(config::log_thread_wait_in_millis);
    log_thread_running = true;
    while(log_thread_running) {
        std::this_thread::sleep_for(wait_time);
        logger::all.flush();
    }

    logger::all.flush();

    return nullptr;
}

void init_log_thread(const char *filename) {
    int log_filedescriptor = open(filename, O_RDWR | O_APPEND | O_CREAT, O_SYNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if ( log_filedescriptor < 0 ) {
        std::cerr << "Failed to open file " << filename << ", error " << errno << ", " << strerror(errno) << std::endl;
    }

    logger::all.set_fd(log_filedescriptor);

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
}


template <size_t N>
constexpr void write_string(char *&pStr, const char (&message)[N]) {
    constexpr size_t n = N-1;
    std::copy(message, message + n, pStr);
    pStr += n;
}

void createLogsString(logger_logs_entry_read &logEntry, char *pStr) {
    const uint8_t *data_args = logEntry.arguments;

    add_time_to_string_helper(pStr, logEntry.timestamp);

    *pStr++ = ':';

    switch(logEntry.id) {
        default:
            assert(true);
#define LOGGER_ENTRY(x, y, m, z) case log_t::x: writeLogsText(#y, sizeof(#y) - 1, pStr); break;
            LOGGER_LOG_LIST
#undef LOGGER_ENTRY
    }

    *pStr++ = ':';

    const char *id_str = get_log_id_string(logEntry.id);
    writeLogsText(id_str, pStr);
    const char *desc_str = get_log_description(logEntry.id);

    *pStr++ = ':';

    formatstring_state state = formatstring_state::COPY;
    formatstring_modifier lenght_specifier = formatstring_modifier::NONE;
    while(*desc_str) {
        char c = *desc_str++;
        switch (state)
        {
        case formatstring_state::COPY:
            switch (c)
            {
            case '%':
                state = formatstring_state::SPECIFIER;
                lenght_specifier = formatstring_modifier::NONE;
                break;
            
            default:
                *pStr++ = c;
                break;
            }
            break;

        case formatstring_state::SPECIFIER:
            switch (c)
            {
            case '%':
                state = formatstring_state::COPY;
                *pStr++ = c;
                break;
            
            case 'd':
            case 'i': {
                switch(lenght_specifier) {
                case formatstring_modifier::h: integerToStringHelper<int16_t>(pStr, data_args); break;
                case formatstring_modifier::hh: integerToStringHelper<int8_t>(pStr, data_args); break;
                case formatstring_modifier::NONE: integerToStringHelper<int32_t>(pStr, data_args); break;
                case formatstring_modifier::l:
                case formatstring_modifier::ll: integerToStringHelper<int64_t>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'u': {
                switch(lenght_specifier) {
                case formatstring_modifier::NONE: integerToStringHelper<uint32_t>(pStr, data_args); break;
                case formatstring_modifier::h: integerToStringHelper<uint16_t>(pStr, data_args); break;
                case formatstring_modifier::hh: integerToStringHelper<uint8_t>(pStr, data_args); break;
                case formatstring_modifier::l:
                case formatstring_modifier::ll: integerToStringHelper<uint64_t>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'o': {
                switch(lenght_specifier) {
                case formatstring_modifier::NONE: integerToStringHelper<uint32_t, 8>(pStr, data_args); break;
                case formatstring_modifier::h: integerToStringHelper<uint16_t, 8>(pStr, data_args); break;
                case formatstring_modifier::hh: integerToStringHelper<uint8_t, 8>(pStr, data_args); break;
                case formatstring_modifier::l:
                case formatstring_modifier::ll: integerToStringHelper<uint64_t, 8>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'x': {
                switch(lenght_specifier) {
                case formatstring_modifier::NONE: integerToStringHelper<uint32_t, 16>(pStr, data_args); break;
                case formatstring_modifier::h: integerToStringHelper<uint16_t, 16>(pStr, data_args); break;
                case formatstring_modifier::hh: integerToStringHelper<uint8_t, 16>(pStr, data_args); break;
                case formatstring_modifier::l:
                case formatstring_modifier::ll: integerToStringHelper<uint64_t, 16>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'X': {
                switch(lenght_specifier) {
                case formatstring_modifier::NONE: integerToStringHelper<uint32_t, 16, number_case::upper>(pStr, data_args); break;
                case formatstring_modifier::h: integerToStringHelper<uint16_t, 16, number_case::upper>(pStr, data_args); break;
                case formatstring_modifier::hh: integerToStringHelper<uint8_t, 16, number_case::upper>(pStr, data_args); break;
                case formatstring_modifier::l:
                case formatstring_modifier::ll: integerToStringHelper<uint64_t, 16, number_case::upper>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'f': {
                switch(lenght_specifier) {
                default:
                case formatstring_modifier::NONE: floatToStringHelper<float>(pStr, data_args); break;
                case formatstring_modifier::l: floatToStringHelper<double>(pStr, data_args); break;
                };
                state = formatstring_state::COPY;
                break;
            }

            case 'c':
                *pStr++ = *data_args++;
                break;
            
            case 'v':
                state = formatstring_state::SPECIFIER_CUSTOM;
                break;

            // Specifiers
            case 'h':
                if (lenght_specifier == formatstring_modifier::h)
                    lenght_specifier = formatstring_modifier::hh;
                else lenght_specifier = formatstring_modifier::h;
                break;
            
            case 'l':
                if (lenght_specifier == formatstring_modifier::l)
                    lenght_specifier = formatstring_modifier::ll;
                else lenght_specifier = formatstring_modifier::l;
                break;
            
            default: // Junk character we just ignoring it
                state = formatstring_state::COPY;
                break;
            } // switch (c)
            break; // case formatstring_state::MODIFIER:
        
        case formatstring_state::SPECIFIER_CUSTOM:
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

ssize_t log_read_helper(int fd, void *buf, size_t n, bool bwait = false) {
    ssize_t read_size = read(fd, (void *)buf, n);
    while(read_size == 0) {
        if (!bwait) return 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        read_size = read(fd, (void *)buf, n);
    }

    if (read_size != n) {
        std::cerr << "Read failure error: " << errno 
            << ", read_size: " << read_size 
            << ", requested_size" << n
            << ", bwait: " << bwait << std::endl;
        throw exception_t(err_t::LOG_READ_FAILURE);
    }

    return read_size;
}

logger_logs_entry_read *logreader::readnext() {
    uint8_t log_common_mem[sizeof(logger_logs_entry_common)] = {0};
    logger_logs_entry_common *log_common = (logger_logs_entry_common *)log_common_mem;
    auto read_size = log_read_helper(file_descriptor, (void *)log_common_mem, sizeof(logger_logs_entry_common));

    if (read_size == 0) return nullptr;

    size_t data_args_size = get_log_length(log_common->id);

    uint8_t *log_mem = new uint8_t[sizeof(logger_logs_entry_common) + data_args_size];

    logger_logs_entry_read *log_read = (logger_logs_entry_read *)log_mem;
    std::copy(log_common_mem, log_common_mem + sizeof(logger_logs_entry_common), log_mem);

    if (data_args_size != 0)
        log_read_helper(file_descriptor, (void *)(log_mem + read_size), data_args_size, true);

    return log_read;
}

const std::string logreader::readnextstring(bool live) {
    logger_logs_entry_read *logread = nullptr;
    while(true) {
        logger_logs_entry_read *logreadtemp = logreader::readnext();

        if (logreadtemp == nullptr) {
            if (live) {
                if (!priqueue.empty()) {
                    uint64_t current_time = std::chrono::system_clock::now().time_since_epoch().count();
                    if (current_time - last_read_time >= buffer_time_in_nanos * 3) {
                        logread = priqueue.top();
                        priqueue.pop();
                        break;
                    }
                }
            } else {
                if (priqueue.empty()) return std::string();
                logread = priqueue.top();
                priqueue.pop();
                break;
            }
        } else {
            if (priqueue.empty()) {
                priqueue.push(logreadtemp);
                last_read_time = std::chrono::system_clock::now().time_since_epoch().count();
            } else if (std::abs(priqueue.top()->timestamp - logreadtemp->timestamp) < buffer_time_in_nanos) {
                priqueue.push(logreadtemp);
            } else {
                priqueue.push(logreadtemp);
                logread = priqueue.top();
                priqueue.pop();
                break;
            }
            continue;
        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(buffer_time_in_nanos));
    }
    
    createLogsString(*logread, text);

    return std::string(text);
}

} // namespace rohit