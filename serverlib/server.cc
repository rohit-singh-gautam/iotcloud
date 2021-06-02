////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include "server.hh"
#include <iot/message.hh>

void ClientConnectionThreaded::execute(rohit::socket_t client_id) {
    std::cout << "Connection Received from " << client_id.get_peer_ipv6_addr() << std::endl;

    size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    size_t read_buffer_length;
    client_id.read(read_buffer, read_buffer_size, read_buffer_length);

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

    client_id.write(write_buffer, write_buffer_size);
}

int main() try {
    int port = 8080;
    rohit::socketserver<rohit::server_execution_threaded<ClientConnectionThreaded>> serversocket(port);
    std::cout << "Local Address: " << serversocket << std::endl;

    rohit::err_t err = serversocket.execute();
    if (isFailure(err)) {
        std::cout << "Failed with failure " << err << std::endl;
    }

    return EXIT_SUCCESS;
} catch(rohit::exception_t excep) {
    std::cout << excep << std::endl;
    return EXIT_FAILURE;
}