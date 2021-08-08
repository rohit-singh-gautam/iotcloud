cmake_minimum_required(VERSION 3.10)

project(HTTPParserTest)

add_executable(HTTPParserTest httpparsertest.cc)

set(include_common
    ${CMAKE_BINARY_DIR}/httpparser
    ${CMAKE_SOURCE_DIR}/httpparser
    ${CMAKE_BINARY_DIR}/serverlib/include
    ${CMAKE_SOURCE_DIR}/serverlib/include
    ${CMAKE_SOURCE_DIR}/thirdparty/md5
)

set(lib_common 
    HTTPParser
    ServerLibrary
)

include_directories(HTTPParserTest PUBLIC ${include_common})

target_link_libraries(HTTPParserTest PUBLIC ${lib_common})
