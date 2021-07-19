cmake_minimum_required(VERSION 3.10)

project(ServerLibraryTestLog)
project(ServerLibraryTestMemory)
project(ServerLibraryTestIPv6Addr)
project(ServerLibraryTestSocketClient)
project(ServerLibraryTestCrypto)

add_executable(ServerLibraryTestLog testlog.cc)
add_executable(ServerLibraryTestMemory testmemory.cc)
add_executable(ServerLibraryTestIPv6Addr testipv6addr.cc)
add_executable(ServerLibraryTestSocketClient client.cc)
add_executable(ServerLibraryTestCrypto testcrypto.cc)

set(include_common
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../../serverlib/include
    )

set(lib_common ServerLibrary)

include_directories(ServerLibraryTestLog PUBLIC ${include_common})
include_directories(ServerLibraryTestMemory PUBLIC ${include_common})
include_directories(ServerLibraryTestIPv6Addr PUBLIC ${include_common})
include_directories(ServerLibraryTestSocketClient PUBLIC ${include_common})
include_directories(ServerLibraryTestCrypto PUBLIC ${include_common})

target_link_libraries(ServerLibraryTestLog PUBLIC ${lib_common})
target_link_libraries(ServerLibraryTestMemory PUBLIC ${lib_common})
target_link_libraries(ServerLibraryTestIPv6Addr PUBLIC ${lib_common})
target_link_libraries(ServerLibraryTestSocketClient PUBLIC ${lib_common})
target_link_libraries(ServerLibraryTestCrypto PUBLIC ${lib_common})
