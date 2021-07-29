cmake_minimum_required(VERSION 3.10)

project(DeviceServer)

add_executable(DeviceServer
    deviceserver.cc
)

include_directories(DeviceServer PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/../serverlib/include
    ${PROJECT_SOURCE_DIR}/../serverlib/include
    ${PROJECT_SOURCE_DIR}/../thirdparty/SimpleJSON
)

target_link_libraries(DeviceServer PUBLIC
    HTTPParser
    ServerLibrary
)
