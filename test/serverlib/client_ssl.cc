////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/message.hh>
#include <iot/net/serversocket.hh>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>


int main(int argc, char *argv[]) try {
    const char sourceGuid[] = "86512592-6b7b-48c4-8bf3-468501c3d9fa";
    const char destGuid[] = "085c3faf-55ef-4cb5-a170-d216d86d2ea8";

    if (argc !=2) {
        std::cout << "Usage: client [<hostname>]:<port>" << std::endl;
        return EXIT_FAILURE;
    }

    rohit::message_command_t messageCommand(sourceGuid);
    messageCommand.add(rohit::to_guid(destGuid), rohit::operation_t::SWITCH, (rohit::operation_value_internal_type)rohit::operation_t::operation_switch_t::ON);
    
    rohit::ipv6_socket_addr_t ipv6addr = rohit::to_ipv6_socket_addr_t(argv[1]);

    rohit::client_socket_ssl_t client_socket(ipv6addr);
    std::cout << "Local Address: " << client_socket << std::endl;   
    std::cout << "Connected: " << client_socket.get_local_ipv6_addr() << " -> " << client_socket.get_peer_ipv6_addr() << std::endl;
    
    rohit::err_t err = client_socket.write((void*)&messageCommand, messageCommand.length());
    if (isFailure(err)) {
        std::cout << err << std::endl;
        client_socket.close();
        return EXIT_FAILURE;
    }

    size_t read_buffer_size = sizeof(rohit::message_command_t);
    uint8_t read_buffer[read_buffer_size];

    size_t read_buffer_length;
    err = client_socket.read((void *)read_buffer, read_buffer_size, read_buffer_length);
    if (isFailure(err)) {
        std::cout << err << std::endl;
        client_socket.close();
        return EXIT_FAILURE;
    }

    rohit::message_base_t *messageBase = (rohit::message_base_t *)read_buffer;
    std::cout << "------Response Start---------\n" << *messageBase << "------Response End---------\n";

    client_socket.close();

    return EXIT_SUCCESS;
} catch(rohit::exception_t excep) {
    std::cout << excep << std::endl;
    return EXIT_FAILURE;
}