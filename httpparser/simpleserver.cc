// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in)

// [START cloudrun_iotcloud_service]
// [START cloud_run_iotcloud]
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <sstream>
#include "http11driver.hh"


int main() {
    const int port = 8080;
    const int backlog = 60;
    int listening_socket;
    int connected_socket;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_address_size;

    size_t read_buffer_size = 1024;
    char read_buffer[read_buffer_size];


    listening_socket = socket(PF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    memset(server_address.sin_zero, '\0', sizeof(server_address.sin_zero)); 

    bind(listening_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    listen(listening_socket, backlog);

    for(;;) {
        client_address_size = sizeof(client_address);
        connected_socket = accept(listening_socket, (struct sockaddr *)&client_address, &client_address_size);

        size_t read_buffer_length = recv(connected_socket , read_buffer, read_buffer_size, 0);
        std::string request_string(read_buffer, read_buffer_length);
        std::cout << "------Request Start---------\n" << request_string << "\n------Request End---------\n";

        rohit::http11driver driver;
        driver.parse(request_string);
        std::cout << "------Driver Start---------\n" << driver << "\n------Driver End---------\n";

        http_response responseContent(
            http_header::VERSION::VER_1_1,
            200_rc, {
              {http_header::FIELD::Server, "IOTCLOUD"},
              {http_header::FIELD::Content_Type, "application/json"},
            }, 
            std::string("{result:""success""}\n"));
        responseContent.addMD5();
        std::stringstream strResponse;
        strResponse << responseContent;
        std::string strResponseHeader = strResponse.str();

        std::cout << "------Response Start---------\n" << strResponseHeader << "\n------Response End---------\n";

        send(connected_socket, strResponseHeader.c_str(), strResponseHeader.length(), 0);
    }

    return 0;
}
// [END cloud_run_iotcloud]
// [END cloudrun_iotcloud_service]
