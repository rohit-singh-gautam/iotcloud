cmake_minimum_required(VERSION 3.10)

add_subdirectory(serverlib)


project(ServerLibraryTestLog)
project(ServerLibraryTestMemory)
project(ServerLibraryTestIPv6Addr)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(ServerLibraryTestLog testlog.cc)
add_executable(ServerLibraryTestMemory testmemory.cc)
add_executable(ServerLibraryTestIPv6Addr testipv6addr.cc)

set(include_common
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/serverlib/include
    )

set(lib_common
    ServerLibrary
    Threads::Threads
    )

include_directories(ServerLibraryTestLog PUBLIC ${include_common})
include_directories(ServerLibraryTestMemory PUBLIC ${include_common})
include_directories(ServerLibraryTestIPv6Addr PUBLIC ${include_common})

target_link_libraries(ServerLibraryTestLog PUBLIC ${lib_common})
target_link_libraries(ServerLibraryTestMemory PUBLIC ${lib_common})
target_link_libraries(ServerLibraryTestIPv6Addr PUBLIC ${lib_common})


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")