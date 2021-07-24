////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/log.hh>
#include <iot/core/configparser.hh>
#include <iot/core/version.h>
#include <iostream>

int main(int argc, char *argv[]) {
    bool live;
    bool display_version;
    const char *log_file;
    rohit::commandline param_parser(
        "Parse and display logs",
        "Parse and display logs in sorted format",
        {
            {'l', "log_file", "file path", "Path to save log file", log_file, "/var/log/iotcloud/deviceserver.log"},
            {'w', "wait", "Wait mode will wait for more logs", live},
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

    rohit::logreader log_reader(log_file);

    while(true) {
        auto logstr = log_reader.readnextstring(live);
        if (logstr.empty()) break;
        std::cout << logstr << std::endl;
    }
    
    return 0;
}