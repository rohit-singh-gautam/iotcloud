cmake_minimum_required(VERSION 3.10)

project(DeviceServer VERSION 1.0)

add_executable(DeviceServer
    iotserverevent.cc
    deviceserver.cc
    )

configure_file(include/version.h.in include/version.h)

include_directories(DeviceServer PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/../serverlib/include
    ${PROJECT_SOURCE_DIR}/../serverlib/include
    ${PROJECT_SOURCE_DIR}/../thirdparty/SimpleJSON
    )

target_link_libraries(DeviceServer PUBLIC ServerLibrary)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")