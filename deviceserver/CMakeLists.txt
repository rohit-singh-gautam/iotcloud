cmake_minimum_required(VERSION 3.10)

project(DeviceServer)

add_executable(DeviceServer
    deviceserver.cc
    lib/iotfilemapping.cc
    lib/iothttphelper.cc
)

include_directories(DeviceServer PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/serverlib/include
    ${CMAKE_SOURCE_DIR}/serverlib/include
    ${CMAKE_BINARY_DIR}/httpparser/include
    ${CMAKE_SOURCE_DIR}/httpparser/include
    ${CMAKE_SOURCE_DIR}/thirdparty/SimpleJSON
    ${CMAKE_SOURCE_DIR}/thirdparty/md5
)

target_link_libraries(DeviceServer PUBLIC
    HTTPParser
    ServerLibrary
)
