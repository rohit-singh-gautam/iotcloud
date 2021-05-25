#include <iot/message.hh>
#include <iot/serversocket.hh>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(int argc, char *argv[]) try {
    const std::string sourceGuid = "86512592-6b7b-48c4-8bf3-468501c3d9fa";
    const std::string destGuid = "085c3faf-55ef-4cb5-a170-d216d86d2ea8";

    if (argc != 3) {
        std::cout << "Usage: client <hostname> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    rohit::message_command_t messageCommand(sourceGuid);
    messageCommand.add(destGuid, rohit::operation_t::SWITCH, (rohit::operation_value_internal_type)rohit::operation_t::operation_switch_t::ON);

    int portno = atoi(argv[2]);
    
    rohit::ipv6_addr ipv6addr(argv[1], portno);

    rohit::client_socket_t client_socket(ipv6addr);
    std::cout << "Connected: " << client_socket << std::endl;   
    std::cout << "Local Address: " << client_socket.get_local_ipv6_addr() << std::endl;
    
    rohit::error_t err = client_socket.write((void*)&messageCommand, messageCommand.length());
    if (err.isFailure()) {
        std::cout << err << std::endl;
        client_socket.close();
        return EXIT_FAILURE;
    }

    size_t read_buffer_size = sizeof(rohit::message_command_t);
    uint8_t read_buffer[read_buffer_size];

    size_t read_buffer_length;
    err = client_socket.read((void *)read_buffer, read_buffer_size, read_buffer_length);
    if (err.isFailure()) {
        std::cout << err << std::endl;
        client_socket.close();
        return EXIT_FAILURE;
    }

    rohit::message_base_t *messageBase = (rohit::message_base_t *)read_buffer;
    std::cout << "------Response Start---------\n" << *messageBase << "------Response End---------\n";

    err = client_socket.close();
    if (err.isFailure()) {
        std::cout << err << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
} catch(rohit::exception_t excep) {
    std::cout << excep << std::endl;
    return EXIT_FAILURE;
}