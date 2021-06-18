cmake_minimum_required(VERSION 3.10)

project(ServerLibrary VERSION 1.0)

add_library(ServerLibrary
    lib/memory/memory.cc
    lib/net/iotserverevent.cc
    lib/states/event_distributor.cc
    lib/iotmessage.cc
    lib/log.cc
)

configure_file(include/iot/core/version.h.in include/iot/core/version.h)

include_directories(ServerLibrary PUBLIC ${PROJECT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")