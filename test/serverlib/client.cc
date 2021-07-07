////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/message.hh>
#include <iot/net/serversocket.hh>
#include <iot/core/configparser.hh>
#include <iot/core/version.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>

rohit::ipv6_socket_addr_t ipv6addr;
rohit::ipv6_socket_addr_t ipv6addr_ssl;
uint32_t repeat;
uint32_t call_count;
uint32_t wait_time_in_ms;
uint32_t wait_call_in_ms;
uint32_t parallel_count;
bool display_version;
bool use_ssl;

int read_failed = 0;
int write_failed = 0;
int calls = 0;
int connect_count = 0;

void test_deviceserver() {
    const char sourceGuid[] = "86512592-6b7b-48c4-8bf3-468501c3d9fa";
    const char destGuid[] = "085c3faf-55ef-4cb5-a170-d216d86d2ea8";
    rohit::message_command_t messageCommand(sourceGuid);
    messageCommand.add(rohit::to_guid(destGuid), rohit::operation_t::SWITCH, (rohit::operation_value_internal_type)rohit::operation_t::operation_switch_t::ON);

    rohit::client_socket_t client_socket(ipv6addr);
    std::cout << "Local Address: " << client_socket << std::endl;   
    std::cout << "Connected: " << client_socket.get_local_ipv6_addr() << " -> " << client_socket.get_peer_ipv6_addr() << std::endl;
    ++connect_count;

    for(uint32_t call_index = 0; call_index < call_count; ++call_index) {
        ++calls;
        rohit::err_t err = client_socket.write((void*)&messageCommand, messageCommand.length());
        if (isFailure(err)) {
            ++write_failed;
            std::cout << "Write failed " << err << std::endl;
            client_socket.close();
            return;
        }

        size_t read_buffer_size = sizeof(rohit::message_command_t);
        uint8_t read_buffer[read_buffer_size];

        size_t read_buffer_length;
        err = client_socket.read((void *)read_buffer, read_buffer_size, read_buffer_length);
        if (isFailure(err)) {
            ++read_failed;
            std::cout << "Read failed " << err << std::endl;
            client_socket.close();
            return;
        }

        rohit::message_base_t *messageBase = (rohit::message_base_t *)read_buffer;
        std::cout << "------Response Start---------\n" << *messageBase << "------Response End---------\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(wait_call_in_ms));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time_in_ms));
    client_socket.close();
}

void test_deviceserver_ssl() {
    const char sourceGuid[] = "86512592-6b7b-48c4-8bf3-468501c3d9fa";
    const char destGuid[] = "085c3faf-55ef-4cb5-a170-d216d86d2ea8";
    rohit::message_command_t messageCommand(sourceGuid);
    messageCommand.add(rohit::to_guid(destGuid), rohit::operation_t::SWITCH, (rohit::operation_value_internal_type)rohit::operation_t::operation_switch_t::ON);

    rohit::client_socket_ssl_t client_socket(ipv6addr_ssl);
    std::cout << "Local Address: " << client_socket << std::endl;   
    std::cout << "Connected: " << client_socket.get_local_ipv6_addr() << " -> " << client_socket.get_peer_ipv6_addr() << std::endl;
    ++connect_count;

    for(uint32_t call_index = 0; call_index < call_count; ++call_index) {
        ++calls;
        rohit::err_t err = client_socket.write((void*)&messageCommand, messageCommand.length());
        if (isFailure(err)) {
            ++write_failed;
            std::cout << "Write failed " << err << std::endl;
            client_socket.close();
            return;
        }

        size_t read_buffer_size = sizeof(rohit::message_command_t);
        uint8_t read_buffer[read_buffer_size];

        size_t read_buffer_length;
        err = client_socket.read((void *)read_buffer, read_buffer_size, read_buffer_length);
        if (isFailure(err)) {
            ++read_failed;
            std::cout << "Read failed " << err << std::endl;
            client_socket.close();
            return;
        }

        rohit::message_base_t *messageBase = (rohit::message_base_t *)read_buffer;
        std::cout << "------Response Start---------\n" << *messageBase << "------Response End---------\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(wait_call_in_ms));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time_in_ms));
    client_socket.close();
}

void *parallel_test(void *) {
    for(uint32_t repeat_index = 0; repeat_index < repeat; ++repeat_index) {
        if (use_ssl) {
            test_deviceserver_ssl();
        } else {
            test_deviceserver();
        }
    }
    return nullptr;
}

int main(int argc, char *argv[]) try {

    rohit::commandline param_parser(
        "Test client to test DeviceServer",
        "Test client would test DeviceServer by calling it multiple times",
        {
            {'a', "address", "IPV6 address with port", "Device server address e.g. [::1]:8080", ipv6addr},
            {'s', "secure_address", "IPV6 address with port", "Device server SSL address e.g. [::1]:8081", ipv6addr_ssl, rohit::ipv6_socket_addr_t()},
            {"use_ssl", "Device server SSL address", use_ssl},
            {'r', "repeat", "Repeat Count", "Number of time repeat has to be conducted", repeat, 1U},
            {'c', "count", "Call Count", "Number of time to call in each repeat", call_count, 1U},
            {'p', "parallel_count", "number of threads", "Number of parallel thread to be created", parallel_count, 1U},
            {'w', "wait", "time in second", "Time to wait for before terminating", wait_time_in_ms, 1U},
            {"wait_call", "time in millisecond", "Time to wait in between calls", wait_call_in_ms, 1U},
            {'v', "version", "Display version", display_version}
        }
    );

    if (!param_parser.parser(argc, argv)) {
        std::cout << param_parser.usage() << std::endl;
        return EXIT_SUCCESS;
    }

    if (display_version) {
        std::cout << param_parser.get_name() << " " << IOT_VERSION_MAJOR << "." << IOT_VERSION_MINOR << std::endl;
        return EXIT_SUCCESS;
    }

    if (parallel_count == 1) {
        parallel_test(nullptr);
    } else {
        pthread_t pthread[parallel_count];

        for(uint32_t thread_index = 0; thread_index < parallel_count; ++thread_index) {
            pthread_create(&pthread[thread_index], NULL, parallel_test, nullptr);
        }

        for(uint32_t thread_index = 0; thread_index < parallel_count; ++thread_index) {
            pthread_join(pthread[thread_index], nullptr);
        }
    }

    std::cout << "Call count " << calls << std::endl;
    std::cout << "Connect count " << connect_count << std::endl;
    std::cout << "Read failed " << read_failed << std::endl;
    std::cout << "Write failed " << write_failed << std::endl;


    return EXIT_SUCCESS;
} catch(rohit::exception_t excep) {
    std::cout << excep << std::endl;
    return EXIT_FAILURE;
}