cmake_minimum_required(VERSION 3.10)

project(ServerLibraryTestLog)
project(ServerLibraryTestMemory)
project(ServerLibraryTestIPv6Addr)
project(ServerLibraryTestSocketClient)
project(ServerLibraryTestSSLSocketClient)

add_executable(ServerLibraryTestLog testlog.cc)
add_executable(ServerLibraryTestMemory testmemory.cc)
add_executable(ServerLibraryTestIPv6Addr testipv6addr.cc)
add_executable(ServerLibraryTestSocketClient client.cc)
add_executable(ServerLibraryTestSSLSocketClient client_ssl.cc)

set(include_common
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../../serverlib/include
    )

set(lib_common ServerLibrary)

include_directories(ServerLibraryTestLog PUBLIC ${include_common})
include_directories(ServerLibraryTestMemory PUBLIC ${include_common})
include_directories(ServerLibraryTestIPv6Addr PUBLIC ${include_common})
include_directories(ServerLibraryTestSocketClient PUBLIC ${include_common})
include_directories(ServerLibraryTestSSLSocketClient PUBLIC ${include_common})

target_link_libraries(ServerLibraryTestLog PUBLIC ${lib_common})
target_link_libraries(ServerLibraryTestMemory PUBLIC ${lib_common})
target_link_libraries(ServerLibraryTestIPv6Addr PUBLIC ${lib_common})
target_link_libraries(ServerLibraryTestSocketClient PUBLIC ${lib_common})
target_link_libraries(ServerLibraryTestSSLSocketClient PUBLIC ${lib_common})


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")