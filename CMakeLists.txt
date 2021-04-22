# Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in)

# [START cloudrun_iotcloud_makelist]
cmake_minimum_required(VERSION 3.10)

# Define the project name and where to report bugs.
set(PACKAGE_BUGREPORT
    "https://github.com/rohit-singh-gautam/iotcloud/issues")
project(iotcloud-singh-org-in CXX C)

# Configure the Compiler options, we will be using C++17 features.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.66 REQUIRED COMPONENTS program_options)
find_package(Threads)

add_executable(iotcloud iotcloud.cc iotcloud_session_handler.cc)
target_link_libraries(
  iotcloud PRIVATE Boost::headers Boost::program_options
                          Threads::Threads)

include(GNUInstallDirs)
install(TARGETS iotcloud RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
# [END cloudrun_iotcloud_makelist]
