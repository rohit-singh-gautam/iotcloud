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

#include <iot/core/log.hh>
#include <iot/config/message.hh>
#include <iot/core/configparser.hh>
#include <iot/core/version.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <ranges>
#include <string_view>
#include <iostream>
#include <vector>
#include <memory>


using std::operator""sv;

// Return number, -1 if error
int GetNumber(const auto &str) {
    if (str.empty()) return -1;
    int ret {0};
    for(auto ch: str) {
        if (ch <= '0' || ch >= '9') return -1;
        ret = ret*10 + ch - '0';
    }
    return ret;
}

rohit::config_log GetConfigLogFromString(const auto &strConfig) {
    auto module { rohit::module_t::UNKNOWN };
    auto level { rohit::logger_level::DEBUG };
    size_t reading = 0;
    std::string_view delimiter {":"};
    for(auto curr: std::views::split(strConfig, delimiter)) {
        auto currstr { std::string(curr.begin(), curr.end())};
        switch(reading) {
            case 0: {
                ++reading;
                if (currstr.empty()) break;
                auto imodule = GetNumber(currstr);
                if (imodule != -1) {
                    if (imodule >= static_cast<decltype(imodule)>(rohit::module_t::UNKNOWN))
                        throw std::string("Unknown module: " + currstr);
                    module = static_cast<rohit::module_t>(imodule);
                } else if (currstr.compare("all"sv) == 0) {
                    module = rohit::module_t::MAX_MODULE;
                } else {
                    module = rohit::to_module_t(currstr);
                    if (module == rohit::module_t::UNKNOWN)
                        throw std::string("Unknown module: " + currstr);
                }
                break;
            }

            case 1: {
                ++reading;
                if (currstr.empty()) break;
                auto ilevel = GetNumber(currstr);
                if (ilevel != -1) {
                    if (ilevel > static_cast<decltype(ilevel)>(rohit::logger_level::ALERT))
                        throw std::string("Bad log level: " + currstr);
                    level = static_cast<rohit::logger_level>(ilevel);
                } else if (currstr.compare("all"sv) == 0) {
                    level = rohit::logger_level::IGNORE;
                } else {
                    level = rohit::to_logger_level(currstr);
                    if (level > rohit::logger_level::ALERT)
                        throw std::string("Unknown log level: " + currstr);
                }
                break;
            }

            default:
                throw std::string("Unknown module configuration: " + currstr);
        }
    }

    return {module, level};
}

auto GetConfigLogList(const auto &logconfig) {
    std::string_view delimiter {","};
    auto options { std::views::split(logconfig, delimiter) };
    std::vector<rohit::config_log> list {};

    for(auto configstr: options) {
        auto config = GetConfigLogFromString(configstr);
        list.push_back(config);
    }
    return list;
}

int main(int argc, char *argv[]) {
    std::string logconfig {};
    bool listmodule {false};
    bool listlevel {false};
    bool display_version {false};
    bool terminate {false};
    rohit::commandline param_parser(
        "Configure Server",
        "This tool is used to configure all device server",
        {
            {'c', "config", "module:level list", "Comma separated list of module:level, module=all for all module, level can be number or name, e.g. SYSTEM:6 or system:debug", logconfig},
            {'m', "module", "List module that configure", listmodule},
            {'l', "level", "List log level", listlevel},
            {'t', "terminate", "Terminate log server", terminate},
            {'v', "version", "Display version", display_version}
        }
    );

    if (!param_parser.parser(argc, argv)) {
        std::cout << param_parser.usage() << std::endl;
        return EXIT_SUCCESS;
    }

    if (display_version) {
        std::cout << param_parser.get_name() << " " << IOT_VERSION_MAJOR << "." << IOT_VERSION_MINOR << std::endl;
    }

    if (listmodule) {
        std::cout << "List of Modules are: \n";
        size_t index {0};
        for(; index < static_cast<size_t>(rohit::module_t::MAX_MODULE); ++index) {
            std::cout << "ID " << index << " - " << rohit::module_t_string[index] << '\n';
        }
        std::cout << "ID " << index << " - all\n";
    }

    if (listlevel) {
        std::cout << "List of levels are: \n";
        size_t index {0};
        for(auto levelname: rohit::logger_level_string) {
            std::cout << "ID " << index << " - " << levelname << '\n';
            ++index;
        }
    }

    int fifoFD { 0 };

    if (!logconfig.empty() || terminate) {
        // Creating FIFO file descriptor
        if (!std::filesystem::exists(rohit::config::ipc_path)) {
            // Check and create parent folder first
            auto fspath { std::filesystem::path(rohit::config::ipc_path) };
            auto fsparent { fspath.parent_path() };
            std::filesystem::create_directories(fsparent);
            mkfifo(rohit::config::ipc_path, 0660);
        }
        fifoFD = open(rohit::config::ipc_path, O_WRONLY);
    }

    if (!fifoFD) return 0;

    if (!logconfig.empty()) {
        try {
            const auto configlist = GetConfigLogList(logconfig);
            if (configlist.empty()) {
                std::cout << "Bad log configuration format.\n";
            } else {
                auto sendsize { sizeof(rohit::config_t) + sizeof(rohit::config_log) * configlist.size() };
                std::unique_ptr<uint8_t[]> sendmem {new uint8_t[sendsize]};
                auto value { rohit::config_t::CONFIG_LOG };
                auto nextcpy = std::copy(reinterpret_cast<uint8_t *>(&value), reinterpret_cast<uint8_t *>(&value) + sizeof(value), sendmem.get());
                for(auto &conf: logconfig) {
                    nextcpy = std::copy(reinterpret_cast<uint8_t *>(&conf), reinterpret_cast<uint8_t *>(&conf) + sizeof(conf), nextcpy);
                }
                auto ret = write(fifoFD, reinterpret_cast<void *>(sendmem.get()), sendsize);
                if (static_cast<decltype(sendsize)>(ret) == sendsize) {
                    std::cout << "Successfully configured log for Device Server\n";
                }
            }
        } catch(const std::string err) {
            std::cout << "Bad log configuration format " << err << '\n';
        }
    }

    if (terminate) {
        auto value { rohit::config_t::CONFIG_TERMINATE };
        auto ret = write(fifoFD, reinterpret_cast<void *>(&value), sizeof(value));
        if (static_cast<size_t>(ret) == sizeof(value)) {
            std::cout << "Successfully terminated Device Server\n";
        }
    }

    if (fifoFD) close(fifoFD);

    return 0;
}