cmake_minimum_required(VERSION 3.10)

project(DeviceServer)

add_executable(DeviceServer
    iotserverevent.cc
    deviceserver.cc
    )

include_directories(DeviceServer PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../serverlib/include
    )

target_link_libraries(DeviceServer PUBLIC ServerLibrary)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")