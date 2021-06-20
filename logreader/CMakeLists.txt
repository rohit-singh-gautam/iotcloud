
cmake_minimum_required(VERSION 3.10)

project(LogReader)

add_executable(LogReader logreader.cc)

include_directories(LogReader PUBLIC ${PROJECT_SOURCE_DIR}/../serverlib/include)

target_link_libraries(LogReader PUBLIC ServerLibrary)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")