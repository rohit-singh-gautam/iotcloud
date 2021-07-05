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
rohit::ipv6_port_t secure_port(0);
const char *cert_file = nullptr;
const char *prikey_file = nullptr;
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
            {'p', "port", "port address", "Port device server would listen to", port, rohit::ipv6_port_t(0)},
            {'s', "secure_port", "SSL port address", "Port device server would listen to", secure_port, rohit::ipv6_port_t(0)},
            {'k', "cert_file", "certificate file path", "Path to certificate", cert_file},
            {"prikey_file", "primary key file path", "Path to primary key, if not provided cert_file will be used", prikey_file, (const char *)nullptr},
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
rohit::serverevent_ssl<rohit::iotserverevent_ssl> *srvevt_ssl = nullptr;

void destroy_app(int) {
    if (evtdist) {
        evtdist->terminate();
        std::cout << "Destroying IOT" << std::endl;
        rohit::destroy_iot();
    }

    if (srvevt) {
        srvevt->close();
        delete srvevt;
    }

    if (srvevt_ssl) {
        srvevt_ssl->close();
        delete srvevt_ssl;
    }

    evtdist->wait();
    delete evtdist;

    std::cout << "All thread joined" << std::endl;

    exit(1);
}

int main(int argc, char *argv[]) try {
    if (!parse_and_display(argc, argv)) return EXIT_SUCCESS;

    if (port == 0 && secure_port == 0) {
        std::cout << "Atleast one of port or secure_port must be set" << std::endl;
        return EXIT_SUCCESS;
    }

    if (secure_port != 0 && cert_file == nullptr) {
        std::cout << "For secure port cert file is mandatory" << std::endl;
        return EXIT_SUCCESS;
    }

    if (cert_file != nullptr && prikey_file == nullptr) {
        prikey_file = cert_file;
    }

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

    if (port != 0) {
        // Execution
        std::cout << "Creating a server at port " << port << std::endl;
        srvevt = new rohit::serverevent<rohit::iotserverevent>(*evtdist, port);
    }

    if constexpr (rohit::config::enable_ssl) {
        if (secure_port != 0) {
            // Execution
            std::cout << "Creating a SSL server at port " << secure_port << std::endl;
            srvevt_ssl = new rohit::serverevent_ssl<rohit::iotserverevent_ssl>(
                *evtdist,
                secure_port,
                cert_file,
                prikey_file);
        }
    }

    // Wait and terminate
    std::cout << "Waiting for all thread to join" << std::endl;
    evtdist->wait();

    return 0;
} catch (rohit::exception_t e) {
    std::cout << "Exception received " << e << std::endl;
    destroy_app(0);
} catch (...) {
    std::cout << "Exception received " << std::endl;
    destroy_app(0);
}