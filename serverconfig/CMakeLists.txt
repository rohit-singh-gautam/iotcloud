cmake_minimum_required(VERSION 3.22)

project(ServerConfig)

add_executable(ServerConfig
    serverconfig.cc
)

include_directories(DeviceServer PUBLIC
    ${CMAKE_BINARY_DIR}/serverlib/include
    ${CMAKE_SOURCE_DIR}/serverlib/include
)

target_link_libraries(ServerConfig PUBLIC
    ServerLibrary
)
