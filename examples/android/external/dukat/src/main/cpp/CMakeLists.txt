# CMake to include external libraries.
cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")

# Include individual libs
set(lib_src_dir ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(${lib_src_dir}/dukat/src ${lib_src_dir}/dukat/build)
