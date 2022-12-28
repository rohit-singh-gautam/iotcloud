cmake_minimum_required(VERSION 3.10)

#sudo apt install libssl-dev
#sudo apt install libpq-dev
#sudo apt install flex
#sudo apt install bison

project(iotcloud)

# Build Type must be one of: Debug Release RelWithDebInfo MinSizeRel
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(COMPILE_WARNING_AS_ERROR true)
add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-vla)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z")

add_subdirectory(serverlib)
add_subdirectory(httpparser)
add_subdirectory(deviceserver)
add_subdirectory(logreader)
add_subdirectory(test/serverlib)
add_subdirectory(test/httpparser)