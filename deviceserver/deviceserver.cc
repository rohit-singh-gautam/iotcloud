////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iotserverevent.hh>
#include <iot/init.hh>

constexpr int port = 8080;
constexpr const char *log_path = "/tmp/deviceserver.log";

int main() {
    // Initialization
    std::cout << "Opening log file at " << log_path << std::endl;
    rohit::init_iot(log_path);

    std::cout << "Creating event distributor" << std::endl;
    rohit::event_distributor evtdist;

    std::cout << "Waiting for a second" << std::endl;
    sleep(1);

    // Execution
    std::cout << "Creating a server" << std::endl;
    rohit::serverevent<rohit::iotserverevent> srvevt(evtdist, port);

    // Wait and terminate
    std::cout << "Waiting for termination" << std::endl;
    evtdist.wait();

    std::cout << "Destroying IOT" << std::endl;
    rohit::destroy_iot();
    return 0;
}