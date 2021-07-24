////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/memory.hh>
#include <iotserverevent.hh>
#include <iot/message.hh>

namespace rohit {

void iotserverevent::close(thread_context &ctx) {
    int last_peer_id = peer_id;
    if (last_peer_id) {
        peer_id.close();
        ctx.log<log_t::IOT_EVENT_SERVER_CONNECTION_CLOSED>((int)last_peer_id);
    }
}

void iotserverevent::execute(thread_context &ctx, const uint32_t event) {
    if ((event & (EPOLLHUP | EPOLLRDHUP)) != 0) {
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

    if (read_buffer_length > 0) {
        rohit::message_base_t *base = (rohit::message_base_t *)read_buffer;

        std::cout << "------Request Start---------\n" << *base << "\n------Request End---------\n";

        size_t write_buffer_size = 0;
        uint8_t write_buffer[std::max(sizeof(rohit::message_unknown_t), sizeof(rohit::message_success_t))];

        if (*base != rohit::message_code_t::COMMAND) {
            std::cout << "Returning unknown" << std::endl;
            rohit::message_unknown_t unknownMessage;
            write_buffer_size = sizeof(unknownMessage);
            std::copy((uint8_t *)&unknownMessage, (uint8_t *)&unknownMessage + sizeof(unknownMessage), write_buffer);
        } else {
            std::cout << "Returning success" << std::endl;
            rohit::message_success_t successMessage;
            write_buffer_size = sizeof(successMessage);
            std::copy((uint8_t *)&successMessage, (uint8_t *)&successMessage + sizeof(successMessage), write_buffer);
        }

        size_t written_length;
        err = peer_id.write_wait(write_buffer, write_buffer_size, written_length);
        if (isFailure(err)) {
            ctx.log<log_t::IOT_EVENT_SERVER_WRITE_FAILED>(errno);
        }
    }
}

void iotserverevent_ssl::close(thread_context &ctx) {
    int last_peer_id = peer_id;
    if (last_peer_id) {
        peer_id.close();
        ctx.log<log_t::IOT_EVENT_SERVER_CONNECTION_CLOSED>(last_peer_id);
    }
}

void iotserverevent_ssl::execute(thread_context &ctx, const uint32_t event) {
    lock();
    if (peer_id.is_closed()) {
        ctx.log<log_t::IOT_EVENT_SERVER_NULL_SOCKET>();
        return;
    }
    if ((event & (EPOLLHUP | EPOLLRDHUP )) != 0) {
        // TODO: Database has to be update with information that connection is closed
        close(ctx);
        return;
    }
    ctx.log<log_t::IOT_EVENT_SERVER_COMMAND_RECEIVED>(peer_id.get_peer_ipv6_addr());

    std::cout << "--- Request received from " << peer_id.get_peer_ipv6_addr() << std::endl;
    std::cout << "--- Thread received " << pthread_self() << std::endl;
    std::cout << "--- Socket " << (int)peer_id << std::endl;

    size_t read_buffer_size = 1024;
        uint8_t read_buffer[read_buffer_size];
    size_t read_buffer_length;
    auto err = peer_id.read_wait(read_buffer, read_buffer_size, read_buffer_length);
    
    if (isFailure(err)) {
        // Most probably SSL_ERROR_ZERO_RETURN
        std::cout << "Closing SSL peer connection " << std::endl;
        ctx.log<log_t::IOT_EVENT_SERVER_READ_FAILED>(errno);
        unlock();
        return;
    }

    if (read_buffer_length > 0) {
        rohit::message_base_t *base = (rohit::message_base_t *)read_buffer;

        std::cout << "------Request Start---------\n" << *base << "\n------Request End---------\n";

        size_t write_buffer_size = 0;
        uint8_t write_buffer[std::max(sizeof(rohit::message_unknown_t), sizeof(rohit::message_success_t))];

        if (*base != rohit::message_code_t::COMMAND) {
            std::cout << "Returning unknown" << std::endl;
            rohit::message_unknown_t unknownMessage;
            write_buffer_size = sizeof(unknownMessage);
            std::copy((uint8_t *)&unknownMessage, (uint8_t *)&unknownMessage + sizeof(unknownMessage), write_buffer);
        } else {
            std::cout << "Returning success" << std::endl;
            rohit::message_success_t successMessage;
            write_buffer_size = sizeof(successMessage);
            std::copy((uint8_t *)&successMessage, (uint8_t *)&successMessage + sizeof(successMessage), write_buffer);
        }


        std::cout << "Write buffer hex: ";
        for(int temp = 0; temp < write_buffer_size; ++temp) {
            uint8_t value = write_buffer[temp];
            std::cout << rohit::upper_case_numbers[value/16] << rohit::upper_case_numbers[value%16];
        }
        std::cout << std::endl;

        size_t written_length;
        err = peer_id.write_wait(write_buffer, write_buffer_size, written_length);
        if (isFailure(err)) {
            ctx.log<log_t::IOT_EVENT_SERVER_WRITE_FAILED>(errno);
        }
    }
    unlock();
}


} // namespace rohit