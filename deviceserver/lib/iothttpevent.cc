////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iothttpevent.hh>


namespace rohit {

void iothttpsslevent::execute(thread_context &ctx) {
    switch (client_state) {
        case state_t::SOCKET_PEER_ACCEPT: {
            // https://www.openssl.org/docs/man3.0/man3/SSL_set_alpn_protos.html
            // This is entry point of SSL

            auto err = peer_id.accept();
            if (err == err_t::SUCCESS) {
                // First remove from epoll
                ctx.remove_event(peer_id);


                const uint8_t *data;
                size_t data_len;
                peer_id.get_protocol(data, data_len);

                if (strncmp((char *)data, "h2", 2) == 0) {
                    auto http2executor = new iothttp2event<true>(std::move(*this));

                    http2executor->client_state = state_t::HTTP2_NEXT_MAGIC;

                    // Add http2executor to epoll
                    ctx.add_event(http2executor->peer_id, EPOLLIN | EPOLLOUT, http2executor);

                    http2executor->execute(ctx);

                    ctx.delayed_free(this);
                    return;
                } else if (strncmp((char *)data, "http/1.1", 8) == 0) {
                    auto httpexecutor = new iothttpevent<true>(std::move(*this));

                    httpexecutor->client_state = state_t::SOCKET_PEER_READ;

                    // Add http2executor to epoll
                    ctx.add_event(httpexecutor->peer_id, EPOLLIN | EPOLLOUT, httpexecutor);

                    httpexecutor->execute(ctx);

                    ctx.delayed_free(this);
                } else {
                    close(ctx);
                }

            } else if (err != err_t::SOCKET_RETRY) {
                close(ctx);
            }
            break;
        }
        case state_t::SOCKET_PEER_CLOSE: {
            close(ctx);
            break;
        }
        case state_t::SOCKET_PEER_WRITE: {
            write_all(ctx);
            break;
        }
        case state_t::SERVEREVENT_MOVED: {
            // This event is move to HTTP 2.0
            break;
        }
    }
}


} // namespace rohit