#pragma once
#include "core/error.hh"
#include "math.hh"
#include "core/ipv6addr.hh"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstring>

namespace rohit {

constexpr ipv6_socket_addr_t::operator sockaddr_in6() const {
    sockaddr_in6 sockaddr = {};
    sockaddr.sin6_family = AF_INET6;
    sockaddr.sin6_addr.__in6_u.__u6_addr32[0] = addr.addr_32[0];
    sockaddr.sin6_addr.__in6_u.__u6_addr32[1] = addr.addr_32[1];
    sockaddr.sin6_addr.__in6_u.__u6_addr32[2] = addr.addr_32[2];
    sockaddr.sin6_addr.__in6_u.__u6_addr32[3] = addr.addr_32[3];
    sockaddr.sin6_port = port.get_network_port();
    return sockaddr;
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

    inline const ipv6_socket_addr_t get_peer_ipv6_addr() const {
        sockaddr_in6 addr;
        socklen_t len = sizeof(addr);
        getpeername(socket_id, reinterpret_cast<struct sockaddr *>(&addr), &len);

        ipv6_port_t &port = *reinterpret_cast<ipv6_port_t *>(&addr.sin6_port);
        return ipv6_socket_addr_t(&addr.sin6_addr.__in6_u, port);
    }

    inline const ipv6_socket_addr_t get_local_ipv6_addr() const {
        sockaddr_in6 addr;
        socklen_t len = sizeof(addr);
        getsockname(socket_id, (struct sockaddr *)&addr, &len);

        ipv6_port_t &port = *reinterpret_cast<ipv6_port_t *>(&addr.sin6_port);
        return ipv6_socket_addr_t(&addr.sin6_addr.__in6_u, port);
    }

    // Returns local or socket IP address
    inline operator const ipv6_socket_addr_t() const {
        return get_local_ipv6_addr();
    }

};

inline std::ostream& operator<<(std::ostream& os, const socket_t &client_id) {
    return os << client_id.get_local_ipv6_addr();
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
    inline error_t connect(const ipv6_socket_addr_t &ipv6addr) {
        sockaddr_in6 in6_addr = ipv6addr;
        if (::connect(socket_id, (struct sockaddr*)&in6_addr, sizeof(in6_addr)) == 0)
            return error_t::SUCCESS;
        return error_t::socket_connect_ret();
    }

public:
    using socket_t::socket_t;
    client_socket_t(const ipv6_socket_addr_t &ipv6addr) {
        error_t err = connect(ipv6addr);
        if (err.isFailure()) throw exception_t(err);
    }
};


} // namespace rohit