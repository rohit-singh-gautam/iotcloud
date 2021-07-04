cmake_minimum_required(VERSION 3.10)

project(iotcloud)

#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE RelWithDebInfo)
#set(CMAKE_BUILD_TYPE MinSizeRel)

add_subdirectory(serverlib)
add_subdirectory(deviceserver)
add_subdirectory(logreader)
add_subdirectory(test/serverlib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")