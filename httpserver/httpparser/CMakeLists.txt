cmake_minimum_required(VERSION 3.10)

project(HTTPParser)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(FLEX_OUTPUT ${CMAKE_BINARY_DIR}/http11lexer.cc)

add_custom_command(
    OUTPUT ${FLEX_OUTPUT}
    COMMAND ${FLEX_EXECUTABLE}
    --outfile=${FLEX_OUTPUT}
    ${CMAKE_SOURCE_DIR}/http11.l
    COMMENT "Generating ${FLEX_OUTPUT}"
)

set(BISON_OUTPUT ${CMAKE_BINARY_DIR}/http11parser.cc)

add_custom_command(
    OUTPUT ${BISON_OUTPUT}
    COMMAND ${BISON_EXECUTABLE}
    --output=${BISON_OUTPUT}
    -d
    ${CMAKE_SOURCE_DIR}/http11.yy
    COMMENT "Generating ${BISON_OUTPUT}"
)

add_library(HTTPParser
    ${BISON_OUTPUT}
    ${FLEX_OUTPUT}
)

include_directories(HTTPServer PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)


