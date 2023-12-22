/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// This program is free software: you can redistribute it and/or modify it under the terms //
// of the GNU General Public License as published by the Free Software Foundation, either  //
// version 3 of the License, or (at your option) any later version.                        //
//                                                                                         //
// This program is distributed in the hope that it will be useful, but WITHOUT ANY         //
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A         //
// PARTICULAR PURPOSE. See the GNU General Public License for more details.                //
//                                                                                         //
// You should have received a copy of the GNU General Public License along with this       //
// program. If not, see <https://www.gnu.org/licenses/>.                                   //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <iotserverevent.hh>
#include <iothttpevent.hh>
#include <iotfilemapping.hh>
#include <httpfilewatcher.hh>
#include <iot/watcher/helperevent.hh>
#include <iot/init.hh>
#include <iot/core/configparser.hh>
#include <iot/core/version.h>
#include <iot/config/message.hh>
#include <signal.h>
#include <json.hpp>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <filesystem>
#include <memory>
#include <thread>

class DeviceServerParameter {
    std::filesystem::path log_file{ };
    const char *config_folder{ };
    bool display_version{ };
    bool log_debug_mode{ };
    int thread_count{ };

    bool valid{ };

public:
    DeviceServerParameter(int argc, char *argv[]) {
        rohit::commandline param_parser(
            "IOT device server that is used to push data on all the devices",
            "Device server is designed to keep hold on all the IOT devices connected to internet. "
            "All devices share their state with this server and even devices are contolled by this server. ",
            {
                {'l', "log_file", "file path", "Path to save log file", log_file, std::filesystem::path("/tmp/iotcloud/log/deviceserver.log")},
                {'c', "config_folder", "folder path", "Path to configuration folder, it must contain file iot.json and logmodule.json", config_folder, "/etc/iotcloud"},
                {'t', "thread_count", "number of thread", "Number of threads that listen to socket, 0 means number of CPU", thread_count, 0},
                {'v', "version", "Display version", display_version},
                {'d', "debug", "Dumps all the logs, logs file will be very big", log_debug_mode}
            }
        );

        valid = param_parser.parser(argc, argv);
        if (!valid) std::cout << param_parser.usage() << std::endl;

        if (display_version) {
            std::cout << param_parser.get_name() << " " << std::to_string(IOT_VERSION_MAJOR) << "." << std::to_string(IOT_VERSION_MINOR) << std::endl;
        }
    }

    auto IsValid() const { return valid; }
    const auto &GetLogFile() const { return log_file; }
    auto GetConfigurationFolder() const { return config_folder; }
    auto GetIsDisplayVersion() const { return display_version; }
    auto GetIsLogDebugMode() const { return log_debug_mode; }
    auto GetThreadCount() const { return thread_count; }
};

class DeviceServer {
    typedef rohit::serverevent<rohit::iotserverevent<false>, false> serverevent_type;
    typedef rohit::serverevent<rohit::iotserverevent<true>, true> serverevent_ssl_type;
    typedef rohit::serverevent<rohit::iothttpevent<false>, false> httpevent_type;
    typedef rohit::serverevent<rohit::iothttpsslevent, true> httpevent_ssl_type;

    std::unique_ptr<rohit::event_distributor> evtdist;
    std::vector<std::unique_ptr<serverevent_type>> srvevts;
    std::vector<std::unique_ptr<serverevent_ssl_type>> srvevts_ssl;
    std::vector<std::unique_ptr<httpevent_type>> srvhttpevts;
    std::vector<std::unique_ptr<httpevent_ssl_type>> srvhttpevts_ssl;

    std::unique_ptr<rohit::http::httpfilewatcher> ptr_filewatcher;

    const DeviceServerParameter &parameter;

public:
    DeviceServer(const DeviceServerParameter &parameter) : parameter{ parameter } {
        std::cout << "Creating event distributor" << std::endl;
        evtdist.reset(new rohit::event_distributor(parameter.GetThreadCount()));
        evtdist->init();

        ptr_filewatcher.reset(new rohit::http::httpfilewatcher(*evtdist));
        ptr_filewatcher->init();

        const auto str_config_folder = std::string(parameter.GetConfigurationFolder());
        const auto configfile = str_config_folder + "/iot.json";

        // Loading servers
        load_and_execute_config(configfile);
    }

    void Wait() {
        evtdist->wait();
    }
    
    auto IsTerminated() { return evtdist->isTerminated(); }

    void destroy_app() {
        if (evtdist) {
            evtdist->terminate();
            std::cout << "Destroying IOT" << std::endl;
            rohit::destroy_iot();
        }

        for(auto &srvevt: srvevts) {
            srvevt->close();
        }

        for(auto &srvevt_ssl: srvevts_ssl) {
            srvevt_ssl->close();
        }

        for(auto &srvhttpevt: srvhttpevts) {
            srvhttpevt->close();
        }

        for(auto &srvhttpevt_ssl: srvhttpevts_ssl) {
            srvhttpevt_ssl->close();
        }

        std::cout << "All thread joined" << std::endl;
    }

private:
    const std::string load_config_string(const char *const configfile) {
        int fd = open(configfile, O_RDONLY);
        if ( fd == -1 ) {
            perror("Unable to open file");
            return {};
        }

        struct stat bufstat;
        fstat(fd, &bufstat);

        int size = bufstat.st_size;
        char buffer[size];

        [[maybe_unused]] auto read_size = read(fd, buffer, size);
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
                    new serverevent_type(port);
                srvevt->init(*evtdist);
                srvevts.emplace_back(srvevt);
            } else if (TYPE == "ssl") {
                const auto cert_file = server["CertFile"].ToString();
                const auto prikey_file_temp = server["PrikeyFile"].ToString();
                const auto prikey_file = !prikey_file_temp.empty() ? prikey_file_temp : cert_file;

                std::cout << "Creating a SSL server at port " << port << ", cert: " << cert_file << ", pri: " << prikey_file <<  std::endl;
                auto srvevt_ssl = new serverevent_ssl_type(
                    port,
                    cert_file.c_str(),
                    prikey_file.c_str());
                srvevt_ssl->init(*evtdist);

                srvevts_ssl.emplace_back(srvevt_ssl);
            } else if (TYPE == "http") {
                std::cout << "Creating a HTTP server at port " << port << std::endl;
                auto webfolder = server["Folder"].ToString();
                rohit::http::webfilemap.add_folder(port, webfolder);
                auto srvhttpevt = new httpevent_type(port);
                srvhttpevt->init(*evtdist);
                srvhttpevts.emplace_back(srvhttpevt);

                ptr_filewatcher->add_folder(webfolder);
            } else if (TYPE == "https") {
                const auto cert_file = server["CertFile"].ToString();
                const auto prikey_file_temp = server["PrikeyFile"].ToString();
                const auto prikey_file = !prikey_file_temp.empty() ? prikey_file_temp : cert_file;

                std::cout << "Creating a HTTPS server at port " << port << ", cert: " << cert_file << ", pri: " << prikey_file <<  std::endl;
                auto webfolder = server["Folder"].ToString();
                rohit::http::webfilemap.add_folder(port, webfolder);
                auto srvhttpevt_ssl = new httpevent_ssl_type(
                    port,
                    cert_file.c_str(),
                    prikey_file.c_str());
                srvhttpevt_ssl->init(*evtdist);
                srvhttpevts_ssl.emplace_back(srvhttpevt_ssl);

                ptr_filewatcher->add_folder(webfolder);
            } else{
                std::cout << "Unknown server type " << TYPE << ", skipping creation of this server" << std::endl;
                continue;
            }
        }

        auto mappings = config["Mappings"];
        for(auto mapping: mappings.ArrayRange()) {
            auto type = mapping["Type"].ToString();
            auto maps = mapping["Maps"];
            if (type == "folder") {
                auto folder_list = mapping["Folders"];
                for(auto folder_json: folder_list.ArrayRange()) {
                    auto folder = folder_json.ToString();
                    for(auto map: maps.ArrayRange()) {
                        auto keystr = map["Key"].ToString();
                        auto valuestr = map["Value"].ToString();
                        rohit::http::webfilemap.add_folder_mapping(folder, keystr, valuestr);
                    }
                }
            } else if (type == "extension") {
                auto folder_list = mapping["Folders"];
                for(auto folder_json: folder_list.ArrayRange()) {
                    auto folder = folder_json.ToString();
                    for(auto map: maps.ArrayRange()) {
                        auto keystr = map["Key"].ToString();
                        auto valuestr = map["Value"].ToString();
                        rohit::http::webfilemap.add_content_type_mapping(folder, keystr, valuestr);
                    }
                }
            }
        }

        rohit::http::webfilemap.update_folder();
    }

};

std::function<void()> destroy_app;
std::function<bool()> IsTerminated;

class mqd_t_raii {
    mqd_t mq;
public:
    mqd_t_raii(const mqd_t mq) : mq(mq) { }
    ~mqd_t_raii() {
        mq_close(mq);
        mq_unlink(rohit::config::ipc_key);
    }

    mqd_t& operator=(const mqd_t value) { return mq = value; }
    operator mqd_t() const { return mq; }
};

void configuration_thread_function(std::stop_token stoken) {
    mq_attr attr {0, rohit::config::ipc_queue_size, rohit::config::ipc_message_size, 0, {0}};
    mqd_t_raii mq  { mq_open(rohit::config::ipc_key, O_CREAT | O_RDWR, 0644, &attr) };
    if(mq < 0) {
        // Retrying
        rohit::log<rohit::log_t::CONFIG_SERVER_INIT_FAILED_RETRY>(errno);
        mq = mq_open(rohit::config::ipc_key, O_CREAT | O_RDWR, 0644, nullptr);
        if (mq < 0) {
            rohit::log<rohit::log_t::CONFIG_SERVER_INIT_FAILED>(errno);
            return;
        }
    }
    mq_getattr(mq, &attr);
    char message[attr.mq_msgsize + 1];
    rohit::log<rohit::log_t::CONFIG_SERVER_INIT_SUCCESS>(attr.mq_maxmsg, attr.mq_msgsize);

    std::fill(message, message + attr.mq_msgsize + 1, 0);
    while(!IsTerminated() && !stoken.stop_requested()) {
        unsigned int prio;
        
        auto bytesread = mq_receive(mq, message, attr.mq_msgsize + 1, &prio);
        if (bytesread <= 0) {
            rohit::log<rohit::log_t::CONFIG_SERVER_READ_FAILED>(errno);
            return;
        }

        switch (*reinterpret_cast<rohit::config_t *>(message))
        {
        case rohit::config_t::CONFIG_LOG: {
            auto index {sizeof(rohit::config_t)};
            while(index + sizeof(rohit::config_log) <= static_cast<decltype(index)>(bytesread)) {
                auto ptrlogconf { reinterpret_cast<rohit::config_log *>(message + index) };
                if (ptrlogconf->module >= rohit::module_t::MAX_MODULE) {
                    rohit::log<rohit::log_t::CONFIG_SERVER_LOG_LEVEL_ALL>(ptrlogconf->level);
                    rohit::enabled_log_module.enable_all(ptrlogconf->level);
                } else {
                    rohit::log<rohit::log_t::CONFIG_SERVER_LOG_LEVEL>(ptrlogconf->level, ptrlogconf->module);
                    rohit::enabled_log_module.set_module(ptrlogconf->module, ptrlogconf->level);
                }
                index += sizeof(rohit::config_log);
            }
            break;
        }
        
        case rohit::config_t::CONFIG_TERMINATE:
            rohit::log<rohit::log_t::CONFIG_SERVER_TERMINATE>();
            destroy_app();
            return;
        default:
            break;
        }
        std::fill(message, message + attr.mq_msgsize + 1, 0);
    }
}

void segv_app() {
    rohit::log<rohit::log_t::SEGMENTATION_FAULT>();
    rohit::segv_log_flush();
}

int get_receive_buffer_size(const int fd) {
    int receive_size;
    unsigned int receive_size_size = sizeof(receive_size);
    if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *)&receive_size, &receive_size_size) == -1) {
        throw rohit::exception_t(rohit::err_t::SOCKET_GET_READ_BUFFER_FAILED);
    }
    return receive_size;
}

void set_receive_buffer_size(const int fd, const size_t buffer_size) {
    int send_size = buffer_size;
    unsigned int send_size_size = sizeof(send_size);
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *)&send_size, send_size_size) == -1) {
        throw rohit::exception_t(rohit::err_t::SOCKET_SET_READ_BUFFER_FAILED);
    }
}

int get_send_buffer_size(const int fd) {
    int receive_size;
    unsigned int receive_size_size = sizeof(receive_size);
    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&receive_size, &receive_size_size) == -1) {
        throw rohit::exception_t(rohit::err_t::SOCKET_GET_WRITE_BUFFER_FAILED);
    }
    return receive_size;
}

void set_send_buffer_size(const int fd, const size_t buffer_size) {
    int send_size = buffer_size;
    unsigned int send_size_size = sizeof(send_size);
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&send_size, send_size_size) == -1) {
        throw rohit::exception_t(rohit::err_t::SOCKET_SET_WRITE_BUFFER_FAILED);
    }
}

rohit::err_t check_socket_limits() {
    int temp_fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

    auto original_receive_size = get_receive_buffer_size(temp_fd);
    if (original_receive_size < rohit::config::socket_read_buffer_size) {
        std::cout << "Socket read buffer is " << original_receive_size << " it must be set to " << rohit::config::socket_read_buffer_size << std::endl;
        std::cout << "---- Attempting to set socket read limits" << std::endl;

        set_receive_buffer_size(temp_fd, rohit::config::socket_read_buffer_size);
        auto new_receive_size = get_receive_buffer_size(temp_fd);
        std::cout << "Socket read buffer set to " << new_receive_size << std::endl;
        if (new_receive_size < rohit::config::socket_read_buffer_size) {
            std::cout << "Unable to set receive memory buffer existing" << std::endl;
            std::cout << "Set setting using `sudo sysctl net.core.rmem_max=" << rohit::config::socket_read_buffer_size <<"`" << std::endl;
            return rohit::err_t::SOCKET_SET_READ_BUFFER_FAILED;
        }
    }

    auto original_send_size = get_send_buffer_size(temp_fd);
    if (original_send_size < rohit::config::socket_write_buffer_size) {
        std::cout << "Socket write buffer is "  << original_send_size << " it must be set to " << rohit::config::socket_write_buffer_size << std::endl;
        std::cout << "---- Attempting to set socket write limits" << std::endl;

        set_send_buffer_size(temp_fd, rohit::config::socket_write_buffer_size);
        auto new_send_size = get_send_buffer_size(temp_fd);
        std::cout << "Socket write buffer set to " << new_send_size << std::endl;
        if (new_send_size < rohit::config::socket_write_buffer_size) {
            std::cout << "Unable to set write memory buffer existing" << std::endl;
            std::cout << "Set setting using `sudo sysctl net.core.wmem_max=" << rohit::config::socket_write_buffer_size <<"`" << std::endl;
            return rohit::err_t::SOCKET_SET_WRITE_BUFFER_FAILED;
        }
    }

    return rohit::err_t::SUCCESS;
}

void signal_destroy_app(int, siginfo_t *, void *) {
    destroy_app();

    exit(0);
}

void signal_segmentation_fault(int, siginfo_t *, void *) {
    segv_app();

    std::cout << "Segmentation fault..." << std::endl;
    exit(1);
}

void set_sigaction() {
    struct sigaction sa = { };
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signal_destroy_app;
    sa.sa_flags   = SA_SIGINFO;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    struct sigaction sa_segv = { };
    sigemptyset(&sa_segv.sa_mask);
    sa_segv.sa_sigaction = signal_segmentation_fault;
    sa_segv.sa_flags   = SA_SIGINFO;

    sigaction(SIGSEGV, &sa_segv, nullptr);
}

int main(int argc, char *argv[]) try {
    rohit::log<rohit::log_t::APPLICATION_STARTING>();
    DeviceServerParameter parameter{argc, argv};
    if (!parameter.IsValid() || parameter.GetIsDisplayVersion()) return EXIT_SUCCESS;

    if (isFailure(check_socket_limits())) {
        return EXIT_FAILURE;
    }

    set_sigaction();

    if (parameter.GetIsLogDebugMode()) {
        rohit::enabled_log_module.enable_all();
    }

    // Initialization
    std::cout << "Opening log file at " << parameter.GetLogFile() << std::endl;
    rohit::init_iot(parameter.GetLogFile());

    DeviceServer deviceServer{ parameter };
    destroy_app = [&deviceServer]() {
        deviceServer.destroy_app();
    };
    IsTerminated = [&deviceServer]() {
        return deviceServer.IsTerminated();
    };

    std::jthread conf_thread { configuration_thread_function };

    // Wait and terminate
    std::cout << "Waiting for all thread to join" << std::endl;
    rohit::log<rohit::log_t::APPLICATION_STARTED_SUCCESSFULLY>();
    deviceServer.Wait();
    conf_thread.request_stop();

    return 0;
} catch (rohit::exception_t e) {
    segv_app();
    std::cout << "Exception received " << e << std::endl;
} catch (...) {
    segv_app();
    std::cout << "Exception received " << std::endl;
}
