cmake_minimum_required(VERSION 2.8)
project(dukat)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Libraries
find_package(SDL2 REQUIRED)
# FIXME
set(SDL2_IMAGE_LIBRARIES /usr/lib/x86_64-linux-gnu/libSDL2_image.so)
set(SDL2_MIXER_LIBRARIES /usr/lib/x86_64-linux-gnu/libSDL2_mixer.so)
#find_package(SDL2_Image REQUIRED)
set(BOX2D_LIBRARY /usr/lib/x86_64-linux-gnu/libBox2D.so.2.3.0)

find_package(GLEW REQUIRED)
# Set OpenGL preference
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)
find_package(PNG REQUIRED)

# The following folders will be included
add_subdirectory(src)
add_subdirectory(examples/collision)
add_subdirectory(examples/flocking)
add_subdirectory(examples/framebuffer)
add_subdirectory(examples/fullscreenfx)
add_subdirectory(examples/grid)
add_subdirectory(examples/heatmap)
add_subdirectory(examples/hello)
add_subdirectory(examples/input)
add_subdirectory(examples/lighting)
add_subdirectory(examples/lighting2d)
add_subdirectory(examples/mapgen)
add_subdirectory(examples/modelviewer)
add_subdirectory(examples/octree)
add_subdirectory(examples/particles)
add_subdirectory(examples/room)
add_subdirectory(examples/scenes)
add_subdirectory(examples/scrolling)
add_subdirectory(examples/skydome)
add_subdirectory(examples/sprites)
add_subdirectory(examples/surface)
add_subdirectory(examples/terrain)
add_subdirectory(examples/text)
add_subdirectory(examples/waves)
