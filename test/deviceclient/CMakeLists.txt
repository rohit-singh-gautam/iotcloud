cmake_minimum_required(VERSION 3.22)

project(deviceclient)

set(include_common
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/serverlib/include
    )

add_executable(deviceclient deviceclient.cc)
include_directories(deviceclient PUBLIC ${include_common})
target_link_libraries(deviceclient PUBLIC ServerLibrary)


