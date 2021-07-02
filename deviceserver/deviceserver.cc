////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iotserverevent.hh>
#include <iot/init.hh>
#include <iot/core/configparser.hh>
#include <iot/core/version.h>
#include <signal.h>

constexpr char app_name[] = "deviceserver";
constexpr bool sleep_before_create_server = false;
rohit::ipv6_port_t port(0);
const char *log_file;
const char *config_folder;
bool display_version;

bool parse_and_display(int argc, char *argv[]) {
    rohit::commandline param_parser(
        app_name,
        "IOT device server that is used to push data on all the devices",
        "Device server is designed to keep hold on all the IOT devices connected to internet. "
        "All devices share their state with this server and even devices are contolled by this server. ",
        {
            {'p', "port", "port address", "Port device server would listen to", port, rohit::ipv6_port_t(8080)},
            {'l', "log_file", "file path", "Path to save log file", log_file, "/var/log/iotcloud/deviceserver.log"},
            {'c', "config_folder", "folder path", "Path to configuration folder, it must contain file network and logmodule", config_folder, "/etc/iotcloud"},
            {'v', "version", "Display version", display_version}
        }
    );

    auto ret = param_parser.parser(argc, argv);
    if (!ret) std::cout << param_parser.usage() << std::endl;

    if (display_version) {
        std::cout << app_name << " " << IOT_VERSION_MAJOR << "." << IOT_VERSION_MINOR << std::endl;
        return false;
    }

    return ret;
}

rohit::event_distributor *evtdist = nullptr;
rohit::serverevent<rohit::iotserverevent> *srvevt = nullptr;

void destroy_app(int signum) {
    if (evtdist) {
        evtdist->terminate();
        std::cout << "Destroying IOT" << std::endl;
        rohit::destroy_iot();
    }

    srvevt->close();
    delete srvevt;

    evtdist->wait();
    delete evtdist;

    std::cout << "All thread joined" << std::endl;

    exit(1);
}

int main(int argc, char *argv[]) {
    if (!parse_and_display(argc, argv)) return EXIT_SUCCESS;

    signal(SIGINT, destroy_app);
    signal(SIGTERM, destroy_app);

    rohit::enabled_module.enable_all();

    // Initialization
    std::cout << "Opening log file at " << log_file << std::endl;
    rohit::init_iot(log_file);

    std::cout << "Creating event distributor" << std::endl;
    evtdist = new rohit::event_distributor();

    if constexpr (sleep_before_create_server) {
        std::cout << "Waiting for a second" << std::endl;
        sleep(1);
    }

    // Execution
    std::cout << "Creating a server at port " << port << std::endl;
    srvevt = new rohit::serverevent<rohit::iotserverevent>(*evtdist, port);

    // Wait and terminate
    std::cout << "Waiting for all thread to join" << std::endl;
    evtdist->wait();

    return 0;
}