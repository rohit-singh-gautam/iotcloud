////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/memory.hh>
#include <iotserverevent.hh>
#include <iot/message.hh>

namespace rohit {

void iotserverevent::close(thread_context &ctx) {
    ctx.log<log_t::IOT_EVENT_SERVER_CONNECTION_CLOSED>((int)peer_id);

    const auto to_free = this;
    delete to_free;
}

void iotserverevent::execute(thread_context &ctx, const uint32_t event) {
    if ((event & EPOLLHUP) == EPOLLHUP) {
        // TODO: Database has to be update with information that connection is closed
        close(ctx);
        return;
    }
    ctx.log<log_t::IOT_EVENT_SERVER_COMMAND_RECEIVED>(peer_id.get_peer_ipv6_addr());

    size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    size_t read_buffer_length;
    auto err = peer_id.read(read_buffer, read_buffer_size, read_buffer_length);
    
    if (isFailure(err)) {
        ctx.log<log_t::IOT_EVENT_SERVER_READ_FAILED>(errno);
    }

    if (read_buffer_length == 0) {
        close(ctx);
        return;
    }

    rohit::message_base_t *base = (rohit::message_base_t *)read_buffer;

    std::cout << "------Request Start---------\n" << *base << "\n------Request End---------\n";

    size_t write_buffer_size = 0;
    void *write_buffer;

    if (*base != rohit::message_code_t::COMMAND) {
        rohit::message_unknown_t unknownMessage;
        write_buffer_size = sizeof(unknownMessage);
        write_buffer = (void *)&unknownMessage;
    } else {
        rohit::message_success_t successMessage;
        write_buffer_size = sizeof(successMessage);
        write_buffer = (void *)&successMessage;
    }

    err = peer_id.write(write_buffer, write_buffer_size);
}


} // namespace rohit