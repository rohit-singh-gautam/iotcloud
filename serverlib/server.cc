#include "server.hh"
#include "iotmessage.hh"

void ClientConnectionThreaded::execute(iotcloud::socket_t client_id) {
    std::cout << "Connection Received from " << client_id.get_ipv6_addr().to_string() << std::endl;

    size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    size_t read_buffer_length;
    client_id.read(read_buffer, read_buffer_size, read_buffer_length);

    iotcloud::message_base_t *base = (iotcloud::message_base_t *)read_buffer;

    std::cout << "------Request Start---------\n" << *base << "\n------Request End---------\n";

    size_t write_buffer_size = 0;
    void *write_buffer;

    if (*base != iotcloud::message_code_t::COMMAND) {
        iotcloud::message_unknown_t unknownMessage;
        write_buffer_size = sizeof(unknownMessage);
        write_buffer = (void *)&unknownMessage;
    } else {
        iotcloud::message_success_t successMessage;
        write_buffer_size = sizeof(successMessage);
        write_buffer = (void *)&successMessage;
    }

    client_id.write(write_buffer, write_buffer_size);
}

int main() try {
    int port = 8080;
    iotcloud::socketserver<iotcloud::server_execution_threaded<ClientConnectionThreaded>> serversocket(port);

    iotcloud::error_t err = serversocket.execute();
    if (err.isFailure()) {
        std::cout << "Failed with failure " << err << std::endl;
    }

    return EXIT_SUCCESS;
} catch(iotcloud::exception_t excep) {
    std::cout << excep << std::endl;
    return EXIT_FAILURE;
}