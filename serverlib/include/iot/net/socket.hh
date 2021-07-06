////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/error.hh>
#include <iot/core/math.hh>
#include <iot/core/ipv6addr.hh>
#include <iot/core/log.hh>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>

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
    if (socket_id < 0) {
        glog.log<log_t::SOCKET_CREATE_FAILED>(errno);
        throw exception_t(rohit::error_c::socket_create_ret());
    }

    glog.log<log_t::SOCKET_CREATE_SUCCESS>(socket_id);
    return socket_id;
}

class socket_t {
protected:
    int socket_id;

    inline socket_t() : socket_id(create_socket()) {}
public:
    inline constexpr socket_t(const int socket_id) : socket_id(socket_id) {}
    inline operator int() const { return socket_id; }

    inline void close() {
        int last_socket_id = __sync_lock_test_and_set(&socket_id, 0);
        if (last_socket_id) {
            auto ret = ::close(last_socket_id);
            if (ret == -1) {
                glog.log<log_t::SOCKET_CLOSE_FAILED>(last_socket_id, errno);
            } else {
                glog.log<log_t::SOCKET_CLOSE_SUCCESS>(last_socket_id);
            }
        }
    }

    inline err_t read(void *buf, const size_t buf_size, size_t &read_len) const {
        int ret = ::read(socket_id, buf, buf_size);
        if (ret == -1) return err_t::RECEIVE_FAILURE;
        read_len = ret;
        return err_t::SUCCESS;
    }

    inline err_t write(const void *buf, const size_t send_len) const {
        // TODO: send in part
        int ret = ::write(socket_id, buf, send_len);
        if (ret == -1) return err_t::SEND_FAILURE;
        return err_t::SUCCESS;
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

    inline bool is_null() { return socket_id == 0; }

    inline bool set_non_blocking() {
        int flags = fcntl(socket_id, F_GETFL, 0);
        if (flags != -1) {
            flags |= O_NONBLOCK;
            flags = fcntl(socket_id, F_SETFL, flags);
        }

        return flags != -1;
    }

};

inline std::ostream& operator<<(std::ostream& os, const socket_t &client_id) {
    return os << client_id.get_local_ipv6_addr();
}

class socket_ssl_t : public socket_t {
public:

protected:
    static void init_openssl(bool isclient = false);
    static void cleanup_openssl();

    static int initialize_ssl_count;
    static SSL_CTX *ctx;
    SSL *ssl;

    static SSL_CTX *create_context(bool isClient = false);

    friend void init_iot(const char *logfilename, const int thread_count);
    friend void destroy_iot();

    friend class server_socket_ssl_t;

    inline socket_ssl_t() { }

public:
    inline socket_ssl_t(const int socket_id, SSL *ssl) : socket_t(socket_id), ssl(ssl) { }

    inline err_t read(void *buf, const size_t buf_size, size_t &read_len) const {
        int ret = SSL_read(ssl, buf, buf_size);
        if (ret == -1) return err_t::RECEIVE_FAILURE;
        read_len = ret;
        return err_t::SUCCESS;
    }

    inline err_t write(const void *buf, const size_t send_len) const {
        // TODO: send in part
        int ret = SSL_write(ssl, buf, send_len);
        if (ret == -1) return err_t::SEND_FAILURE;
        return err_t::SUCCESS;
    }

    inline void close() {
        int last_socket_id = __sync_lock_test_and_set(&socket_id, 0);
        if (last_socket_id) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            auto ret = ::close(last_socket_id);
            if (ret == -1) {
                glog.log<log_t::SOCKET_CLOSE_FAILED>(last_socket_id, errno);
            } else {
                glog.log<log_t::SOCKET_CLOSE_SUCCESS>(last_socket_id);
            }
        }
    }

};

class server_socket_t : public socket_t {
public:
    inline server_socket_t(const int port) {
        int enable = 1;
        if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&enable,sizeof(enable)) < 0) {
            close(); 
            throw exception_t(error_c::sockopt_ret());
        }

        struct sockaddr_in6 addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(port);
        addr.sin6_addr = in6addr_any;

        if (bind(socket_id, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close();
            throw exception_t(err_t::BIND_FAILURE);
        }
        glog.log<log_t::SOCKET_BIND_SUCCESS>(socket_id, port);

        if (listen(socket_id, config::socket_backlog) < 0) {
            close();
            throw exception_t(err_t::LISTEN_FAILURE);
        }
        glog.log<log_t::SOCKET_LISTEN_SUCCESS>(socket_id, port);
    }

    inline socket_t accept() {
        auto client_id = ::accept(socket_id, NULL, NULL);
        if (client_id == -1) {
            if (errno == EAGAIN) {
                return 0;
            }
            throw exception_t(err_t::ACCEPT_FAILURE);
        }

        glog.log<log_t::SOCKET_ACCEPT_SUCCESS>(socket_id, client_id);
        return client_id;
    }

};

class server_socket_ssl_t : public server_socket_t {

public:
    inline server_socket_ssl_t(const int port, const char *const cert_file, const char *const prikey_file)
            : server_socket_t(port) {
        auto ctx = socket_ssl_t::ctx;
        socket_ssl_t::init_openssl(false);

        SSL_CTX_set_ecdh_auto(ctx, 1);

        if ( SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0 ) {
            ERR_print_errors_fp(stderr);
            glog.log<log_t::SOCKET_SSL_CERT_LOAD_FAILED>();
            throw exception_t(err_t::SOCKET_SSL_CERTIFICATE_FAILED);
        } else {
            glog.log<log_t::SOCKET_SSL_CERT_LOAD_SUCCESS>();
        }

        if (SSL_CTX_use_PrivateKey_file(ctx, prikey_file, SSL_FILETYPE_PEM) <= 0 ) {
            ERR_print_errors_fp(stderr);
            glog.log<log_t::SOCKET_SSL_PRIKEY_LOAD_FAILED>();
            throw exception_t(err_t::SOCKET_SSL_PRIKEY_FAILED);
        } else {
            glog.log<log_t::SOCKET_SSL_PRIKEY_LOAD_SUCCESS>();
        }
    }

    inline ~server_socket_ssl_t() {
        socket_ssl_t::cleanup_openssl();
    }

    inline socket_ssl_t accept() {
        auto client_id = ::accept(socket_id, NULL, NULL);
        if (client_id == -1) {
            if (errno == EAGAIN) {
                return {0, nullptr};
            }
            throw exception_t(err_t::ACCEPT_FAILURE);
        }

        auto ssl = SSL_new(socket_ssl_t::ctx);
        SSL_set_fd(ssl, client_id);

        auto ssl_ret = SSL_accept(ssl); 
        if (ssl_ret <= 0) {
            ERR_print_errors_fp(stdout);
            glog.log<log_t::SOCKET_SSL_ACCEPT_FAILED>(socket_id, client_id);
            throw exception_t(err_t::ACCEPT_FAILURE);
        }

        glog.log<log_t::SOCKET_SSL_ACCEPT_SUCCESS>(socket_id, client_id);
        return {client_id, ssl};
    }
};

class client_socket_t : public socket_t {
private:
    inline err_t connect(const ipv6_socket_addr_t &ipv6addr) {
        sockaddr_in6 in6_addr = ipv6addr;
        if (::connect(socket_id, (struct sockaddr*)&in6_addr, sizeof(in6_addr)) == 0)
            return err_t::SUCCESS;
        return error_c::socket_connect_ret();
    }

public:
    using socket_t::socket_t;
    client_socket_t(const ipv6_socket_addr_t &ipv6addr) {
        err_t err = connect(ipv6addr);
        if (isFailure(err)) throw exception_t(err);
    }
};

class client_socket_ssl_t : public socket_ssl_t {
private:
    inline err_t connect(const ipv6_socket_addr_t &ipv6addr) {
        sockaddr_in6 in6_addr = ipv6addr;
        if (::connect(socket_id, (struct sockaddr*)&in6_addr, sizeof(in6_addr)) == 0)
            return err_t::SUCCESS;
        return error_c::socket_connect_ret();
    }

public:
    inline client_socket_ssl_t(const int socket_id, SSL *ssl) : socket_ssl_t(socket_id, ssl) {}
    
    inline client_socket_ssl_t(const ipv6_socket_addr_t &ipv6addr) {
        init_openssl(true);
        err_t err = connect(ipv6addr);
        if (isFailure(err)) throw exception_t(err);

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, socket_id);

        if (SSL_connect(ssl) < 0)
        {
            throw exception_t(err_t::SSL_CONNECT_FAILED);
        }
    }

    inline ~client_socket_ssl_t() {
        socket_ssl_t::cleanup_openssl();
    }
};


} // namespace rohit