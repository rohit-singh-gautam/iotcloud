cmake_minimum_required(VERSION 3.22)

#sudo apt install flex
#sudo apt install bison
#sudo apt install generate-ninja

project(iotcloud)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(COMPILE_WARNING_AS_ERROR true)
add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-vla)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z")

add_subdirectory(serverlib)
add_subdirectory(httpparser)
add_subdirectory(deviceserver)
add_subdirectory(logreader)
add_subdirectory(serverconfig)
add_subdirectory(test/serverlib)
add_subdirectory(test/httpparser)
add_subdirectory(test/deviceclient)
