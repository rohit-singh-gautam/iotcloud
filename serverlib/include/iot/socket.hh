#pragma once
#include "core/error.hh"
#include "math.hh"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstring>

namespace rohit {

class ipv6_addr {
private:
    const sockaddr_in6 addr;
    const std::size_t len;

    static inline sockaddr_in6 stringToAddress(const char *addr_str, const uint16_t port) {
        struct sockaddr_in6 server_address;

        server_address.sin6_family = AF_INET6;
        inet_pton(AF_INET6, addr_str, &server_address.sin6_addr);
        server_address.sin6_port = htons(port);
        return server_address;
    }

    inline constexpr ipv6_addr(const sockaddr_in6 &addr, const size_t len) : addr(addr), len(len) { }

    friend class socket_t;
    friend class client_socket_t;

public:
    inline ipv6_addr(const std::string &addr_str, const uint16_t port)
        : addr(stringToAddress(addr_str.c_str(), port)), len(sizeof(addr)) {}
    inline ipv6_addr(const char *addr_str, const uint16_t port) : addr(stringToAddress(addr_str, port)), len(sizeof(addr)) {}

    inline uint16_t getPort() const { return htons(addr.sin6_port); }

    inline size_t copy_string(char *ipv6_str) const {
        // Stored memory for string like "000.000.000.000:00000"
        const size_t displayString_size = INET6_ADDRSTRLEN + 6;
        inet_ntop(AF_INET6, &addr.sin6_addr, ipv6_str, displayString_size);
        size_t count = 0;
        while(*ipv6_str) {
            ++count;
            ++ipv6_str;
        }
        *ipv6_str++ = ':'; count++;
        count += math::integerToString<uint16_t, 10, math::number_case::lower, false>(ipv6_str, getPort());
        return count;
    }

    const std::string to_string() const;

    inline operator const std::string() const { return to_string(); }
};

template <> struct type_length<type_identifier::ipv6_addr_t> { static constexpr size_t const value = sizeof(ipv6_addr); };

inline std::ostream& operator<<(std::ostream& os, const ipv6_addr &ipv6Addr) {
    return os << ipv6Addr.to_string();
}

inline int create_socket() {
    int socket_id = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (socket_id < 0) throw exception_t(rohit::error_t::socket_create_ret());
    return socket_id;
}

class socket_t {
protected:
    const int socket_id;

    inline socket_t() : socket_id(create_socket()) {}
public:
    inline constexpr socket_t(const int socket_id) : socket_id(socket_id) {}
    inline operator int() const { return socket_id; }

    inline error_t close() const {
        auto ret = ::close(socket_id);
        if (ret == -1) return error_t::CLOSE_FAILURE;
        else return error_t::SUCCESS;
    }

    inline error_t read(void *buf, const size_t buf_size, size_t &read_len) const {
        int ret = ::read(socket_id, buf, buf_size);
        if (ret == -1) return error_t::RECEIVE_FAILURE;
        read_len = ret;
        return error_t::SUCCESS;
    }

    inline error_t write(const void *buf, const size_t send_len) const {
        // TODO: send in part
        int ret = ::write(socket_id, buf, send_len);
        if (ret == -1) return error_t::SEND_FAILURE;
        return error_t::SUCCESS;
    }

    inline const ipv6_addr get_ipv6_addr() const {
        sockaddr_in6 addr;
        socklen_t len = sizeof(addr);
        getpeername(socket_id, (struct sockaddr *)&addr, &len);
        return ipv6_addr(addr, len);
    }

    inline const ipv6_addr get_local_ipv6_addr() const {
        sockaddr_in6 addr;
        socklen_t len = sizeof(addr);
        getsockname(socket_id, (struct sockaddr *)&addr, &len);
        return ipv6_addr(addr, len);
    }
};

inline std::ostream& operator<<(std::ostream& os, const socket_t &client_id) {
    return os << client_id.get_ipv6_addr();
}

class server_socket_t : public socket_t {
public:
    server_socket_t(const int port, const int backlog = 5) {
        int enable = 1;
        if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&enable,sizeof(enable)) < 0) {
            close();
            throw exception_t(error_t::sockopt_ret());
        }

        struct sockaddr_in6 addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(port);
        addr.sin6_addr = in6addr_any;

        if (bind(socket_id, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close();
            throw exception_t(exception_t::BIND_FAILURE);
        }

        if (listen(socket_id, backlog) < 0) {
            close();
            throw exception_t(exception_t::LISTEN_FAILURE);
        }
    }

};

class client_socket_t : public socket_t {
private:
    inline error_t connect(const ipv6_addr &ipv6addr) {
        if (::connect(socket_id, (struct sockaddr*)&ipv6addr.addr, ipv6addr.len) == 0)
            return error_t::SUCCESS;
        return error_t::socket_connect_ret();
    }

public:
    using socket_t::socket_t;
    client_socket_t(const ipv6_addr &ipv6addr) {
        error_t err = connect(ipv6addr);
        if (err.isFailure()) throw exception_t(err);
    }
};


} // namespace rohit