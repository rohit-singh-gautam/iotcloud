////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iotserverevent.hh>
#include <iothttpevent.hh>
#include <iot/init.hh>
#include <iot/core/configparser.hh>
#include <iot/core/version.h>
#include <signal.h>
#include <json.hpp>

const char *log_file;
const char *config_folder;
bool display_version;
bool log_debug_mode;
int thread_count;

bool parse_and_display(int argc, char *argv[]) {
    rohit::commandline param_parser(
        "IOT device server that is used to push data on all the devices",
        "Device server is designed to keep hold on all the IOT devices connected to internet. "
        "All devices share their state with this server and even devices are contolled by this server. ",
        {
            {'l', "log_file", "file path", "Path to save log file", log_file, "/var/log/iotcloud/deviceserver.log"},
            {'c', "config_folder", "folder path", "Path to configuration folder, it must contain file iot.json and logmodule.json", config_folder, "/etc/iotcloud"},
            {'t', "thread_count", "number of thread", "Number of threads that listen to socket, 0 means number of CPU", thread_count, 0},
            {'v', "version", "Display version", display_version},
            {'d', "debug", "Dumps all the logs, logs file will be very big", log_debug_mode}
        }
    );

    auto ret = param_parser.parser(argc, argv);
    if (!ret) std::cout << param_parser.usage() << std::endl;

    if (display_version) {
        std::cout << param_parser.get_name() << " " << IOT_VERSION_MAJOR << "." << IOT_VERSION_MINOR << std::endl;
        return false;
    }

    return ret;
}

typedef rohit::serverevent<rohit::iotserverevent<false>, false> serverevent_type;
typedef rohit::serverevent<rohit::iotserverevent<true>, true> serverevent_ssl_type;
typedef rohit::serverevent<rohit::iothttpevent<false>, false> httpevent_type;
typedef rohit::serverevent<rohit::iothttpevent<true>, true> httpevent_ssl_type;

rohit::event_distributor *evtdist = nullptr;
std::vector<serverevent_type *> srvevts;
std::vector<serverevent_ssl_type *> srvevts_ssl;
std::vector<httpevent_type *> srvhttpevts;
std::vector<httpevent_ssl_type *> srvhttpevts_ssl;

const std::string load_config_string(const char *const configfile) {
    int fd = open(configfile, O_RDONLY);
    if ( fd == -1 ) {
        perror("Unable to open file");
        return 0;
    }

    struct stat bufstat;
    fstat(fd, &bufstat);

    int size = bufstat.st_size;
    char buffer[size];

    auto read_size = read(fd, buffer, size);
    std::string buffer_str = std::string(buffer);
    
    return buffer_str;
}

void load_and_execute_config(const std::string configfile) {
    std::string buffer = load_config_string(configfile.c_str());

    auto config = json::JSON::Load(buffer);

    auto servers = config["servers"];

    for(auto server: servers.ArrayRange()) {
        const auto IP = server["IP"].ToString();
        const auto TYPE = server["TYPE"].ToString();
        const auto port = server["port"].ToInt();

        if (IP != "*") {
            std::cout << "Only * is supported for IP address, skipping creation of this server" << std::endl;
            continue;
        }

        if (port == 0) {
            std::cout << "Port not provided or it is 0, skipping creation of this server" << std::endl;
            continue;
        }

        if (TYPE == "simple") {
            std::cout << "Creating a server at port " << port << std::endl;
            auto srvevt =
                new serverevent_type(*evtdist, port);
            srvevt->init();
            srvevts.push_back(srvevt);
        } else if (TYPE == "ssl") {
            const auto cert_file = server["CertFile"].ToString();
            const auto prikey_file_temp = server["PrikeyFile"].ToString();
            const auto prikey_file = !prikey_file_temp.empty() ? prikey_file_temp : cert_file;

            std::cout << "Creating a SSL server at port " << port << ", cert: " << cert_file << ", pri: " << prikey_file <<  std::endl;
            auto srvevt_ssl = new serverevent_ssl_type(
                *evtdist,
                port,
                cert_file.c_str(),
                prikey_file.c_str());
            srvevt_ssl->init();

            srvevts_ssl.push_back(srvevt_ssl);
        } else if (TYPE == "http") {
            std::cout << "Creating a HTTP server at port " << port << std::endl;
            auto srvhttpevt =
                new httpevent_type(*evtdist, port);
            srvhttpevt->init();
            srvhttpevts.push_back(srvhttpevt);
        } else if (TYPE == "https") {
            const auto cert_file = server["CertFile"].ToString();
            const auto prikey_file_temp = server["PrikeyFile"].ToString();
            const auto prikey_file = !prikey_file_temp.empty() ? prikey_file_temp : cert_file;

            std::cout << "Creating a HTTPS server at port " << port << ", cert: " << cert_file << ", pri: " << prikey_file <<  std::endl;
            auto srvhttpevt_ssl = new httpevent_ssl_type(
                *evtdist,
                port,
                cert_file.c_str(),
                prikey_file.c_str());
            srvhttpevt_ssl->init();

            srvhttpevts_ssl.push_back(srvhttpevt_ssl);
        } else{
            std::cout << "Unknown server type " << TYPE << ", skipping creation of this server" << std::endl;
            continue;
        }
    }
}

void destroy_app() {
    if (evtdist) {
        evtdist->terminate();
        std::cout << "Destroying IOT" << std::endl;
        rohit::destroy_iot();
    }

    for(auto srvevt: srvevts) {
        srvevt->close();
        delete srvevt;
    }

    for(auto srvevt_ssl: srvevts_ssl) {
        srvevt_ssl->close();
        delete srvevt_ssl;
    }

    for(auto srvhttpevt: srvhttpevts) {
        srvhttpevt->close();
        delete srvhttpevt;
    }

    for(auto srvhttpevt_ssl: srvhttpevts_ssl) {
        srvhttpevt_ssl->close();
        delete srvhttpevt_ssl;
    }

    evtdist->wait();
    delete evtdist;

    std::cout << "All thread joined" << std::endl;
}

void segv_app() {
    rohit::glog.log<rohit::log_t::SEGMENTATION_FAULT>();
    rohit::segv_log_flush();
}

void signal_destroy_app(int signal, siginfo_t *siginfo, void *args) {
    destroy_app();

    exit(0);
}

void signal_segmentation_fault(int signal, siginfo_t *siginfo, void *args) {
    segv_app();

    std::cout << "Segmentation fault..." << std::endl;
    exit(1);
}

void set_sigaction() {
    struct sigaction sa = { 0 };
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signal_destroy_app;
    sa.sa_flags   = SA_SIGINFO;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    struct sigaction sa_segv = { 0 };
    sigemptyset(&sa_segv.sa_mask);
    sa_segv.sa_sigaction = signal_segmentation_fault;
    sa_segv.sa_flags   = SA_SIGINFO;

    sigaction(SIGSEGV, &sa_segv, nullptr);
}

int main(int argc, char *argv[]) try {
    if (!parse_and_display(argc, argv)) return EXIT_SUCCESS;

    set_sigaction();

    if (log_debug_mode) {
        rohit::enabled_log_module.enable_all();
    }

    // Initialization
    std::cout << "Opening log file at " << log_file << std::endl;
    rohit::init_iot(log_file);

    std::cout << "Creating event distributor" << std::endl;
    evtdist = new rohit::event_distributor(thread_count);
    evtdist->init();

    const auto str_config_folder = std::string(config_folder);
    const auto configfile = str_config_folder + "/iot.json";

    // Loading servers
    load_and_execute_config(configfile);

    // Wait and terminate
    std::cout << "Waiting for all thread to join" << std::endl;
    evtdist->wait();

    return 0;
} catch (rohit::exception_t e) {
    segv_app();
    std::cout << "Exception received " << e << std::endl;
} catch (...) {
    segv_app();
    std::cout << "Exception received " << std::endl;
}