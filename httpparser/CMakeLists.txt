cmake_minimum_required(VERSION 3.10)

project(HTTPParser)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

BISON_TARGET(HTTPParser
    ${PROJECT_SOURCE_DIR}/http11.yy
    ${PROJECT_BINARY_DIR}/http11parser.cc)
FLEX_TARGET(HTTPParser ${PROJECT_SOURCE_DIR}/http11.l ${PROJECT_BINARY_DIR}/http11lexer.cc)

add_library(HTTPParser
    ${BISON_HTTPParser_OUTPUTS}
    ${FLEX_HTTPParser_OUTPUTS}
    http11driver.cc
    http11.cc
)

include_directories(HTTPParser PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/serverlib/include
    ${CMAKE_SOURCE_DIR}/serverlib/include
    ${CMAKE_SOURCE_DIR}/thirdparty/md5
)

target_link_libraries(HTTPParser PUBLIC
    ServerLibrary
)

