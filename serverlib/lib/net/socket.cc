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

unsigned char proto_list[] = {
        2, 'h', '2',
        8, 'h', 't', 't', 'p', '/', '1', '.', '1'
    };

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

    SSL_CTX_set_read_ahead(ctx, 1);
    SSL_CTX_set_alpn_select_cb(ctx, alpn_cb, nullptr);
    SSL_CTX_set_next_protos_advertised_cb(ctx, alpn_negotiate_cb, nullptr);
    SSL_CTX_set_alpn_protos(ctx, proto_list, sizeof(proto_list));

    SSL_CTX_set_session_id_context(ctx, (const unsigned char *) "HTTP", strlen("HTTP"));
    SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER);
    SSL_CTX_sess_set_cache_size(ctx, 1);

    return ctx;
}

int socket_ssl_t::alpn_cb(
            SSL *s, const unsigned char **out, unsigned char *outlen,
            const unsigned char *in, unsigned int inlen, void *arg)
{
    // TODO: Check input syntax

    if (SSL_select_next_proto
        ((unsigned char **)out, outlen, proto_list, sizeof(proto_list), in,
         inlen) != OPENSSL_NPN_NEGOTIATED) {
        return SSL_TLSEXT_ERR_ALERT_FATAL;
    }

    return SSL_TLSEXT_ERR_OK;
}

int socket_ssl_t::alpn_negotiate_cb(SSL *ssl,
                                const unsigned char **out,
                                unsigned int *outlen,
                                void *arg)
{
    *out = proto_list;
    *outlen = sizeof(proto_list);
    return SSL_TLSEXT_ERR_OK;
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