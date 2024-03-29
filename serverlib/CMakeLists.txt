cmake_minimum_required(VERSION 3.22)

project(ServerLibrary VERSION 1.0)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(PostgreSQL_INCLUDE_DIR /usr/include/postgresql)
set(PostgreSQL_LIBRARY_DIR /usr/lib/postgresql)
set(PostgreSQL_TYPE_INCLUDE_DIR /usr/include/postgresql)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

add_library(ServerLibrary
    lib/memory/memory.cc
    lib/security/crypto.cc
    lib/net/socket.cc
    lib/core/configparser.cc
    lib/states/event_distributor.cc
    lib/init.cc
    lib/message.cc
    lib/log.cc
    lib/memory_helper.cc
)

configure_file(include/iot/core/version.h.in include/iot/core/version.h)

include_directories(ServerLibrary PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ServerLibrary PUBLIC 
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)
