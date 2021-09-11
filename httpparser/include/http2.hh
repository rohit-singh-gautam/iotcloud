////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/types.hh>
#include <hpack.hh>
#include <iostream>

namespace rohit {
namespace http::v2 {

constexpr char connection_preface[] = {
        'P', 'R', 'I', ' ', '*', ' ', 
        'H', 'T', 'T', 'P', '/', '2', '.', '0',
        '\r', '\n', '\r', '\n', 'S', 'M', '\r', '\n', '\r', '\n'};


// This is definition of frame
// Can be called as frame header
struct frame {
    enum class type_t : uint8_t {
        DATA            = 0x00,
        HEADERS         = 0x01,
        PRIORITY        = 0x02,
        RST_STREAM      = 0x03,
        SETTINGS        = 0x04,
        PUSH_PROMISE    = 0x05,
        PING            = 0x06,
        GOAWAY          = 0x07,
        WINDOW_UPDATE   = 0x08,
        CONTINUATION    = 0x09,
    };

    static constexpr const char * get_string(const type_t type) {
        switch(type) {
            case type_t::DATA: return "DATA";
            case type_t::HEADERS: return "HEADERS";
            case type_t::PRIORITY: return "PRIORITY";
            case type_t::RST_STREAM: return "RST STREAM";
            case type_t::SETTINGS: return "SETTINGS";
            case type_t::PUSH_PROMISE: return "PUSH PROMISE";
            case type_t::PING: return "PING";
            case type_t::GOAWAY: return "GOAWAY";
            case type_t::WINDOW_UPDATE: return "WINDOW UPDATE";
            case type_t::CONTINUATION: return "CONTINUATION";
            default: return "BAD TYPE";
        }
    }

    enum class flags_t : uint8_t {
        NONE        = 0x00,
        END_STREAM  = 0x01,
        END_HEADERS = 0x04,
        PADDED      = 0x08,
        PRIORITY    = 0x20,
    };

    static constexpr const char * get_string(const flags_t flag) {
        switch(flag) {
            case flags_t::NONE: return "NONE";
            case flags_t::END_STREAM: return "END STREAM";
            case flags_t::END_HEADERS: return "END HEADERS";
            case flags_t::PADDED: return "PADDED";
            case flags_t::PRIORITY: return "PRIORITY";
            default: return "BAD FLAG";
        }
    }

    enum class error_t : uint32_t {
        NO_ERROR            = 0x00,
        PROTOCOL_ERROR      = 0x01,
        INTERNAL_ERROR      = 0x02,
        FLOW_CONTROL_ERROR  = 0x03,
        SETTINGS_TIMEOUT    = 0x04,
        STREAM_CLOSED       = 0x05,
        FRAME_SIZE_ERROR    = 0x06,
        REFUSED_STREAM      = 0x07,
        CANCEL              = 0x08,
        COMPRESSION_ERROR   = 0x09,
        CONNECT_ERROR       = 0x0a,
        ENHANCE_YOUR_CALM   = 0x0b,
        INADEQUATE_SECURITY = 0x0c,
        HTTP_1_1_REQUIRED   = 0x0d,
    };

    static constexpr const char * get_string(const error_t err) {
        switch(err) {
            case error_t::NO_ERROR: return "NO ERROR";
            case error_t::PROTOCOL_ERROR: return "PROTOCOL ERROR";
            case error_t::INTERNAL_ERROR: return "INTERNAL ERROR";
            case error_t::FLOW_CONTROL_ERROR: return "FLOW CONTROL ERROR";
            case error_t::SETTINGS_TIMEOUT: return "SETTINGS TIMEOUT";
            case error_t::STREAM_CLOSED: return "STREAM CLOSED";
            case error_t::FRAME_SIZE_ERROR: return "FRAME SIZE ERROR";
            case error_t::REFUSED_STREAM: return "REFUSED STREAM";
            case error_t::CANCEL: return "CANCEL";
            case error_t::COMPRESSION_ERROR: return "COMPRESSION ERROR";
            case error_t::CONNECT_ERROR: return "CONNECT ERROR";
            case error_t::ENHANCE_YOUR_CALM: return "ENHANCE YOUR CALM";
            case error_t::INADEQUATE_SECURITY: return "INADEQUATE SECURITY";
            case error_t::HTTP_1_1_REQUIRED: return "HTTP 1.1 REQUIRED";
            default: return "UNKNOWN ERROR";
        }
    }

private:
    /* big endian format
    uint32_t    length:24;
    type_t      type;
    flags_t     flags;
    uint32_t    reserved:1;
    uint32_t    stream_identifier:31; */

    // Little endian format
    uint32_t    length:24;
    type_t      type;
    flags_t     flags;
    uint32_t    stream_identifier:31;
    uint32_t    reserved:1;

public:

    constexpr const char * get_string() const { return get_string(type); }

    constexpr uint32_t get_length() const { return changeEndian(length << 8); }
    constexpr type_t get_type() const { return type; }
    constexpr flags_t get_flags() const { return flags; }
    constexpr bool contains(const flags_t flag) const { return (flags_t)((uint8_t)flags & (uint8_t)flag) == flag; }
    constexpr uint32_t get_stream_identifier() const { return changeEndian(stream_identifier); }

    constexpr frame() {}

    constexpr frame(
                uint32_t    length,
                type_t      type,
                flags_t     flags,
                uint32_t    stream_identifier)
            : length(changeEndian(length) >> 8),
              type(type),
              flags(flags),
              reserved(0),
              stream_identifier(changeEndian(stream_identifier)) {}
} __attribute__((packed)); // struct frame

inline std::ostream& operator<<(std::ostream& os, const frame::type_t &http2frametype) {
    return os << (uint16_t)http2frametype << ':' << frame::get_string(http2frametype);
}

inline std::ostream& operator<<(std::ostream& os, const frame::flags_t &http2frameflag) {
    os << (uint16_t)http2frameflag;
    uint8_t flags = (uint8_t)http2frameflag;
    
    if (flags) os << ':';

    while(flags) {
        uint8_t next_flags = flags & (flags - 1);
        const frame::flags_t flg = (frame::flags_t)(flags^next_flags);
        os << '(' << (uint16_t)flg << ')';
        os << frame::get_string(flg);
        if (next_flags) os << ",";
        flags = next_flags;
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const frame::error_t &http2frameerror) {
    return os << (uint32_t)http2frameerror << ':' << frame::get_string(http2frameerror);
}

inline std::ostream& operator<<(std::ostream& os, const frame &http2frame) {
    return os << "{L:" << http2frame.get_length() << ", T:" << http2frame.get_type()
        << ", F:" << http2frame.get_flags() << ", ID:" << http2frame.get_stream_identifier() << '}';
}

struct header {
private:
    /* Big endian
    uint32_t    E:1;
    uint32_t    stream_dependency:31; */

    // Little endian
    uint32_t    stream_dependency:31;
    uint32_t    E:1;
    //uint8_t     weight;
public:
    constexpr bool get_execlusive() const { return E; }
    constexpr const char *get_exclusive_string() const { return E ? "Exclusive" : ""; }
    constexpr uint32_t get_stream_dependency() const { return changeEndian(stream_dependency); }
    //constexpr uint8_t get_weight() const { return weight; }
} __attribute__((packed)); // struct header

inline std::ostream& operator<<(std::ostream& os, const header &header) {
    return os << '[' << (header.get_execlusive() ? "Exclusive," : "") << "S:" << header.get_stream_dependency()
              << ']';

}

struct settings {
    enum class identifier_t : uint16_t {
        SETTINGS_HEADER_TABLE_SIZE          = 0x01,
        SETTINGS_ENABLE_PUSH                = 0x02,
        SETTINGS_MAX_CONCURRENT_STREAMS     = 0x03,
        SETTINGS_INITIAL_WINDOW_SIZE        = 0x04,
        SETTINGS_MAX_FRAME_SIZE             = 0x05,
        SETTINGS_MAX_HEADER_LIST_SIZE       = 0x06,
    };

    static constexpr const char * get_string(const identifier_t id) {
        switch(id) {
            case identifier_t::SETTINGS_HEADER_TABLE_SIZE: return "HEADER TABLE SIZE";
            case identifier_t::SETTINGS_ENABLE_PUSH: return "ENABLE PUSH";
            case identifier_t::SETTINGS_MAX_CONCURRENT_STREAMS: return "MAX CONCURRENT STREAMS";
            case identifier_t::SETTINGS_INITIAL_WINDOW_SIZE: return "INITIAL WINDOW SIZE";
            case identifier_t::SETTINGS_MAX_FRAME_SIZE: return "MAX FRAME SIZE";
            case identifier_t::SETTINGS_MAX_HEADER_LIST_SIZE: return "MAX HEADER LIST SIZE";
            default: return "UNKNOWN SETTINGS";
        }
    }

private:
    uint16_t identifier;
    uint32_t value;

public:
    constexpr identifier_t get_identifier() const { return (identifier_t)changeEndian(identifier); }
    constexpr uint32_t get_value() const { return changeEndian(value); }

} __attribute__((packed)); // struct settings

inline std::ostream& operator<<(std::ostream& os, const settings::identifier_t &id) {
    return os << settings::get_string(id);
}

inline std::ostream& operator<<(std::ostream& os, const settings &settings) {
    return os << "[" << settings.get_identifier() << "," << settings.get_value() << "]";
}


struct ping {
    enum class flags_t : uint8_t {
        REQUEST = 0x00,
        ACK     = 0x01,
    };
    uint8_t     opaque_data[64] = "ROHIT HTTP SERVER PING                                         ";
} __attribute__((packed));

struct goaway {
private:
    /* Big endian
    uint32_t        reserved:1;
    uint32_t        last_stream_id:31;
    uint32_t        error_code; */

    // Little endian
    uint32_t        last_stream_id:31;
    uint32_t        reserved:1;
    uint32_t        error_code;

public:
    constexpr uint32_t get_last_stream_id() const { return changeEndian(last_stream_id);}
    constexpr frame::error_t get_error_code() const { return (frame::error_t)changeEndian(error_code); }

    // Additional data
} __attribute__((packed));

inline std::ostream& operator<<(std::ostream& os, const goaway &goaway) {
    return os << "[" << goaway.get_last_stream_id() << "," << goaway.get_error_code() << "]";
}

struct window_update {
private:
    /* Big endian
    uint32_t    reserved:1;
    uint32_t    window_size_increment:31; */

    // Little endian
    uint32_t    window_size_increment:31;
    uint32_t    reserved:1;

public:
    uint32_t get_window_size_increment() const { return changeEndian(window_size_increment); }
} __attribute__((packed));

inline std::ostream& operator<<(std::ostream& os, const window_update &windowupdate) {
    return os << '[' << windowupdate.get_window_size_increment() << ']';
}

inline void displaymem(std::ostream& os, const uint8_t *pstart, const uint8_t *const pend) {
    os << '[';
    for(;pstart < pend; ++pstart) {
        os << lower_case_numbers[*pstart / 16] << lower_case_numbers[*pstart % 16];
    }
    os << ']' << std::endl;
}

inline void displaybinary(std::ostream& os, const uint8_t *indata, const size_t indata_size) {
    const uint8_t *const enddata = indata + indata_size;

    os << "S: " << indata_size << ';';

    while(indata + sizeof(frame) <= enddata) {
        const frame *pframe = (frame *)indata;
        indata += sizeof(frame);
        os << *pframe;
        switch(pframe->get_type()) {
            case frame::type_t::HEADERS: {
                const uint8_t *pstart = indata;
                const uint8_t *pend = pstart + pframe->get_length();
                os << '[';
                for(;pstart < pend; ++pstart) {
                    os << lower_case_numbers[*pstart / 16] << lower_case_numbers[*pstart % 16];
                }
                os << ']';
                pstart = indata + sizeof(header);
                for(;pstart < pend; ++pstart) {
                    if (*pstart >= 32 && *pstart <= 127) {
                        os << (char)*pstart;
                    } else {
                        os << '.';
                    }
                }
                os << ']';
                break;
            }
            case frame::type_t::SETTINGS: {
                const uint8_t *pstart = indata;
                const uint8_t *pend = pstart + pframe->get_length();
                while(pstart + sizeof(settings) <= pend) {
                    const settings *psettings = (settings *)pstart;
                    os << *psettings;
                    pstart += sizeof(settings);
                }
                break;
            }
            case frame::type_t::GOAWAY: {
                const uint8_t *pstart = indata;
                const goaway *pgoaway = (goaway *)pstart;
                std::string addinginfo((char *)pstart + sizeof(goaway), pframe->get_length() - sizeof(goaway));
                os << *pgoaway << ":" << addinginfo;
                break;
            }
            case frame::type_t::WINDOW_UPDATE: {
                const uint8_t *pstart = indata;
                const window_update *pwindowupdate = (window_update *)pstart;
                os << *pwindowupdate;
                break;
            }
        }
        indata += pframe->get_length();
        os << ';';
    }

    os << std::endl;
}

class header_request : public http_header_request {
public:
    uint32_t stream_identifier;
    uint8_t weight;
    frame::error_t error;

    // Doubly linked list
    header_request *next;
    header_request *previous;

    inline header_request(uint32_t stream_identifier = 0, uint8_t weight = 16)
                :   stream_identifier(stream_identifier),
                    weight(weight),
                    error(frame::error_t::NO_ERROR),
                    next(nullptr), previous(nullptr) {}

    void parse_header(const uint8_t *pstart, const uint8_t *pend, dynamic_table_t &dynamic_table) {
        while(pstart < pend) {
            if ((*pstart & 0x80) == 0x80) {
                // rfc7541 # 6.1 Indexed Header Field Representation
                size_t index = *pstart & 0x7f; ++pstart;
                // Dynamic table check is internal
                auto header = index < 62 ? static_table[index] : dynamic_table[index - 62];
                fields.insert(header);
            } else if (*pstart == 0x40) {
                ++pstart;
                auto field = get_header_field(pstart);
                auto value = get_header_string(pstart);
                auto header = std::make_pair(field, value);
                dynamic_table.insert(header);
                fields.insert(header);
            } else if ((*pstart & 0xc0) == 0x40) {
                // rfc7541 # 6.2.1 Literal Header Field with Incremental Indexing
                size_t index = *pstart & 0x3f; ++pstart;
                auto header_for_field = index < 62 ? static_table[index] : dynamic_table[index - 62];
                auto value = get_header_string(pstart);
                auto header = std::make_pair(header_for_field.first, value);
                dynamic_table.insert(header);
                fields.insert(header);
            } else if (*pstart == 0x00) {
                ++pstart;
                auto field = get_header_field(pstart);
                auto value = get_header_string(pstart);
                auto header = std::make_pair(field, value);
                fields.insert(header);
            } else if ((*pstart & 0xf0) == 0x00) {
                size_t index = *pstart & 0x0f; ++pstart;
                auto header_for_field = index < 62 ? static_table[index] : dynamic_table[index - 62];
                auto value = get_header_string(pstart);
                auto header = std::make_pair(header_for_field.first, value);
                fields.insert(header);
            } else if (*pstart == 0x10) {
                ++pstart;
                auto field = get_header_field(pstart);
                auto value = get_header_string(pstart);
                auto header = std::make_pair(field, value);
                fields.insert(header);
            } else if ((*pstart & 0xf0) == 0x10) {
                size_t index = *pstart & 0x0f; ++pstart;
                auto header_for_field = index < 62 ? static_table[index] : dynamic_table[index - 62];
                auto value = get_header_string(pstart);
                auto header = std::make_pair(header_for_field.first, value);
                fields.insert(header);
            } else if ((*pstart &0xe0) == 0x20) {
                size_t index = *pstart & 0x1f; ++pstart;
                dynamic_table.update_size(index);
            }
        }

        // TODO: Update Method
        // Update version
        version = VERSION::VER_2;
        auto method_itr = fields.find(http_header::FIELD::Method);
        if (method_itr != fields.end()) {
            method = get_header_method(method_itr->second);
        } else {
            method = http_header_request::METHOD::IGNORE_THIS;
        }
    }

    void set_error(const frame::error_t error) {
        this->error = error;
    }
}; // class header_request

class request {
    //Stream_count
    dynamic_table_t &dynamic_table; // This will be share by the multiple request in a connection
    header_request *first;
    header_request *last;
    std::unordered_map<uint32_t, header_request *> header_map;
    // TODO: implement priority

public:
    inline request(dynamic_table_t &dynamic_table) : dynamic_table(dynamic_table), first(nullptr), last(nullptr), header_map() {}
    inline ~request() {
        // This class allocate, hence will be freeing it
        while(first) {
            header_request *next = first->next;
            delete first;
            first = next;
        }
    }

    inline void insert(header_request *pheader, uint32_t stream_dependency) {
        if (stream_dependency != 0) {
            auto header_itr = header_map.find(stream_dependency);
            header_request *request;
            if (header_itr == header_map.end()) {
                // Dependency stream is not present
                // We are creating dummy header and appending it at the end
                request = new header_request(stream_dependency);
                header_map.insert(std::make_pair(stream_dependency, request));
                last->next = request;
                request->next = pheader;
                last = pheader;
            } else {
                request = header_itr->second;
                pheader->next = request->next;
                request->next = pheader;
                if (request == last) {
                    last = pheader;
                }
            }
        } else {
            // Just appending at the end
            last->next = pheader;
            last = pheader;
        }

        header_map.insert(std::make_pair(pheader->stream_identifier, pheader));
    }

    err_t parse(const uint8_t *pstart, const uint8_t *pend) {
        while(pstart + sizeof(frame) <= pend) {
            const frame *pframe = (frame *)pstart;
            pstart += sizeof(frame);
            const uint8_t padded_bytes = pframe->contains(frame::flags_t::PADDED) ? *pstart++ : 0;

            switch(pframe->get_type()) {
            case frame::type_t::HEADERS: {
                // rfc7540 - 6.2.  HEADERS
                const uint8_t *pstart_ = pstart;
                pstart += pframe->get_length();
                uint32_t stream_dependency;
                if (pframe->contains(frame::flags_t::PRIORITY)) {
                    stream_dependency = changeEndian(*(uint32_t *)pstart_);
                    bool exclusive = (stream_dependency & 0x80000000) == 0x80000000;
                    stream_dependency &= 0x7fffffff;
                    pstart_ += sizeof(uint32_t);
                    uint8_t weight = *pstart_;
                    pstart_++;
                } else {
                    stream_dependency = 0x00;
                }

                const uint32_t stream_identifier = pframe->get_stream_identifier();
                if (stream_identifier == 0x00) {
                    // PROTOCOL_ERROR we will stop parsing
                    return err_t::HTTP2_PARSER_PROTOCOL_ERROR;
                }

                auto header_itr = header_map.find(stream_identifier);
                header_request *request;
                if (header_itr == header_map.end()) {
                    request = new header_request();
                    insert(request, stream_dependency);
                } else {
                    request = header_itr->second;
                }

                request->parse_header(pstart_, pstart - padded_bytes, dynamic_table);
                break;
            }
            case frame::type_t::PRIORITY: {
                // rfc7540 - 6.3.  PRIORITY
                // We will just ignore this
                const uint8_t *pstart_ = pstart;
                pstart += pframe->get_length();
                uint32_t stream_dependency = changeEndian(*(uint32_t *)pstart_);
                bool exclusive = (stream_dependency & 0x80000000) == 0x80000000;
                stream_dependency &= 0x7fffffff;
                pstart_ += sizeof(uint32_t);
                uint8_t weight = *pstart_;
                pstart_++;
                break;
            }
            case frame::type_t::RST_STREAM: {
                const uint8_t *pstart_ = pstart;
                pstart += pframe->get_length();

                const uint32_t error_code = *(uint32_t *)pstart_;
                
                const uint32_t stream_identifier = pframe->get_stream_identifier();
                if (stream_identifier == 0x00) {
                    // PROTOCOL_ERROR we will stop parsing
                    return err_t::HTTP2_PARSER_PROTOCOL_ERROR;
                }

                auto header_itr = header_map.find(stream_identifier);
                header_request *request;
                if (header_itr == header_map.end()) {
                    request = new header_request();
                    insert(request, 0x00);
                } else {
                    request = header_itr->second;
                }

                request->set_error((frame::error_t)error_code);
                break;
            }
            case frame::type_t::SETTINGS: {
                const uint8_t *pstart_ = pstart;
                pstart += pframe->get_length();
                while(pstart_ + sizeof(settings) <= pstart) {
                    const settings *psettings = (settings *)pstart_;
                    pstart_ += sizeof(settings);
                    // Ignoring this
                }
                break;
            }
            case frame::type_t::PUSH_PROMISE: {
                const uint8_t *pstart_ = pstart;
                pstart += pframe->get_length();
                // Will ignore this
                break;
            }
            case frame::type_t::PING: {
                const uint8_t *pstart_ = pstart;
                pstart += pframe->get_length();
                // TODO: Implement ping
                break;
            }
            default:
                pstart += pframe->get_length();
                break;
            }
        }
        return err_t::SUCCESS;
    }
}; // class request


} // namespace http::v2

} // namespace rohit