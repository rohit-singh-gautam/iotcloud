cmake_minimum_required(VERSION 3.10)

project(HTTPParser)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

make_directory(${PROJECT_BINARY_DIR}/include)

BISON_TARGET(HTTPParserBison
    ${PROJECT_SOURCE_DIR}/http11.yy
    ${PROJECT_BINARY_DIR}/http11parser.cc
    DEFINES_FILE ${PROJECT_BINARY_DIR}/include/http11parser.hh)
FLEX_TARGET(HTTPParserFlex
    ${PROJECT_SOURCE_DIR}/http11.l
    ${PROJECT_BINARY_DIR}/http11lexer.cc)

ADD_FLEX_BISON_DEPENDENCY(HTTPParserFlex HTTPParserBison)

add_library(HTTPParser
    ${BISON_HTTPParserBison_OUTPUTS}
    ${FLEX_HTTPParserFlex_OUTPUTS}
    lib/http11driver.cc
    lib/http11.cc
    lib/hpack.cc
)

ADD_CUSTOM_COMMAND(TARGET HTTPParser
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy  ${PROJECT_BINARY_DIR}/location.hh ${PROJECT_BINARY_DIR}/include/location.hh
)

include_directories(HTTPParser PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/serverlib/include
    ${CMAKE_SOURCE_DIR}/serverlib/include
    ${CMAKE_SOURCE_DIR}/thirdparty/md5
)

target_link_libraries(HTTPParser PUBLIC
    ServerLibrary
)

