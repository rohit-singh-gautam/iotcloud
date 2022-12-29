
cmake_minimum_required(VERSION 3.22)

project(LogReader)

add_executable(LogReader logreader.cc)

include_directories(LogReader PUBLIC
    ${CMAKE_BINARY_DIR}/serverlib/include
    ${CMAKE_SOURCE_DIR}/serverlib/include
)

target_link_libraries(LogReader PUBLIC ServerLibrary)
