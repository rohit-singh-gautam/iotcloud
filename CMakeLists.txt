cmake_minimum_required(VERSION 3.10)

project(iotcloud)

# Build Type must be one of: Debug Release RelWithDebInfo MinSizeRel
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z")

add_subdirectory(serverlib)
add_subdirectory(httpparser)
add_subdirectory(deviceserver)
add_subdirectory(logreader)
add_subdirectory(test/serverlib)
add_subdirectory(test/httpparser)