cmake_minimum_required(VERSION 3.12)
project(dukat)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

option(BOX2D_SUPPORT "Build dukat with Box2D physics support" OFF)

# Libraries
find_package(SDL2 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

if(BOX2D_SUPPORT)
    find_library(BOX2D_LIBRARY NAMES Box2D box2d)
    if(NOT BOX2D_LIBRARY)
        message(FATAL_ERROR "BOX2D_SUPPORT is set but the Box2D library was not found")
    endif()
endif()

find_package(GLEW REQUIRED)
# Set OpenGL preference
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)
find_package(PNG REQUIRED)
find_package(CURL REQUIRED)

# Common link libraries for example applications
set(DUKAT_EXAMPLE_LIBS
    dukat
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LINK_LIBRARIES}
    ${SDL2_MIXER_LINK_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${X11_Xext_LIB}
    ${CMAKE_THREAD_LIBS_INIT}
)

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
add_subdirectory(examples/layers)
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
