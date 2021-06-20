////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/log.hh>
#include <iostream>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <log filename>" << std::endl;
        return 0;
    }

    rohit::logreader log_reader(argv[1]);

    while(true) {
        auto logstr = log_reader.readnext();
        std::cout << logstr << std::endl;
    }
    
    return 0;
}