cmake_minimum_required(VERSION 3.10)

add_subdirectory(serverlib)


project(ServerLibraryTestLog)
#project(ServerLibraryTestMemory)
#project(ServerLibraryTestIPv6Addr)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(ServerLibraryTestLog testlog.cc)

include_directories(ServerLibraryTestLog PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/serverlib/include
    )

target_link_libraries(ServerLibraryTestLog PUBLIC
    ServerLibrary
    Threads::Threads
    )


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")