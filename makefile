
CMAKE_BUILD_DIR := build
CMAKE_SOURCE_DIR := .

$(CMAKE_BUILD_DIR)/Makefile: $(CMAKE_SOURCE_DIR)/CMakeLists.txt
	cmake -S $(<D) -B $(@D)
