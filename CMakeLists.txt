cmake_minimum_required(VERSION 2.8)
project(dukat)

set(CMAKE_CXX_FLAGS "-std=c++14")
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Libraries
# FIXME
set(SDL2_LIBRARY /usr/lib/x86_64-linux-gnu/libSDL2-2.0.so)
set(SDL2_IMAGE_LIBRARIES /usr/lib/x86_64-linux-gnu/libSDL2_image-2.0.so)

#find_package(SDL2 REQUIRED)
#find_package(SDL2_Image REQUIRED)
#find_package(SDL2_ttf REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

# The following folders will be included
add_subdirectory(src/dukat)
add_subdirectory(examples/surface)
add_subdirectory(examples/text)
add_subdirectory(examples/room)
