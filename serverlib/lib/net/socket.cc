////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <openssl/ssl.h>
#include <iot/net/socket.hh>
#include <iot/core/log.hh>
#include <openssl/err.h>

namespace rohit {

int socket_ssl_t::initialize_ssl_count = 0;
SSL_CTX *socket_ssl_t::ctx;

SSL_CTX *socket_ssl_t::create_context(bool isclient)
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    if (isclient) method = TLS_client_method();
    else method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
	    ERR_print_errors_fp(stderr);
        throw exception_t(err_t::SOCKET_SSL_CONTEXT_FAILED);
    }

    return ctx;
}

void socket_ssl_t::init_openssl(bool isclient)
{
    if (!initialize_ssl_count) {
        glog.log<log_t::SOCKET_SSL_INITIALIZE>();
        if (isclient) SSL_library_init();
        SSL_load_error_strings();	
        OpenSSL_add_all_algorithms();
        ctx = create_context(isclient);
        ++initialize_ssl_count;
    } else {
        ++initialize_ssl_count;
        glog.log<log_t::SOCKET_SSL_INITIALIZE_ATTEMPT>(initialize_ssl_count);
    }
}

void socket_ssl_t::cleanup_openssl()
{
    --initialize_ssl_count;

    if (initialize_ssl_count == 0) {
        glog.log<log_t::SOCKET_SSL_CLEANUP>();
        SSL_CTX_free(ctx);
        ctx = nullptr;
        EVP_cleanup();
    } else {
        glog.log<log_t::SOCKET_SSL_CLEANUP_ATTEMPT>(initialize_ssl_count);
    }

    assert(initialize_ssl_count >= 0);
}


}