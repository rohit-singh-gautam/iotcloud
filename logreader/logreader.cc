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
#include <iot/core/configparser.hh>
#include <iot/core/version.h>
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>

void delete_log_file(const char *filename) {
    std::filesystem::path path { filename };
    std::filesystem::remove(path);
}

void wait_for_creation(const char *filename) {
    std::filesystem::path path { filename };

    while(!std::filesystem::exists(path)) {
        // Wait for one second
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }
}

int main(int argc, char *argv[]) {
    bool live;
    bool clean;
    bool display_version;
    const char *log_file;
    rohit::commandline param_parser(
        "Parse and display logs",
        "Parse and display logs in sorted format",
        {
            {'l', "log_file", "file path", "Path to save log file", log_file, "/tmp/log/iotcloud/deviceserver.log"},
            {'w', "wait", "Wait mode will wait for more logs", live},
            {'c', "clean", "Delete log file", clean},
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

    if (clean) delete_log_file(log_file);

    if (live) wait_for_creation(log_file);

    rohit::logreader log_reader(log_file);

    while(true) {
        auto logstr = log_reader.readnextstring(live);
        if (logstr.empty()) break;
        std::cout << logstr << std::endl;
    }
    
    return 0;
}