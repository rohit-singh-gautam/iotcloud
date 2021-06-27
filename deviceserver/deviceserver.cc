////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iotserverevent.hh>
#include <iot/init.hh>
#include <iot/core/configparser.hh>

constexpr bool sleep_before_create_server = false;
rohit::ipv6_port_t port(0);
const char *log_path;

rohit::commandline param_parser(
    "deviceserver",
    "IOT device server that is used to push data on all the devices",
    "Device server is designed to keep hold on all the IOT devices connected to internet. "
    "All devices share their state with this server and even devices are contolled by this server. ",
    {
        {'p', "port", "Port device server would listen to", port, rohit::ipv6_port_t(8080)},
        {'l', "log_path", "Path to save log file", log_path, "/tmp/deviceserver.log"}
    }
);


int main(int argc, char *argv[]) {
    if (!param_parser.parser(argc, argv)) {
        std::cout << param_parser.usage() << std::endl;
        return EXIT_SUCCESS;
    }

    // Initialization
    std::cout << "Opening log file at " << log_path << std::endl;
    rohit::init_iot(log_path);

    std::cout << "Creating event distributor" << std::endl;
    rohit::event_distributor evtdist;

    if constexpr (sleep_before_create_server) {
        std::cout << "Waiting for a second" << std::endl;
        sleep(1);
    }

    // Execution
    std::cout << "Creating a server at port " << port << std::endl;
    rohit::serverevent<rohit::iotserverevent> srvevt(evtdist, port);

    // Wait and terminate
    std::cout << "Waiting for termination" << std::endl;
    evtdist.wait();

    std::cout << "Destroying IOT" << std::endl;
    rohit::destroy_iot();
    return 0;
}