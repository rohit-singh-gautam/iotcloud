cmake_minimum_required(VERSION 3.10)

project(iotcloud)

add_subdirectory(serverlib)
add_subdirectory(deviceserver)
add_subdirectory(logreader)
add_subdirectory(test/serverlib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")