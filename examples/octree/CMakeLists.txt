include_directories(../../include)

option(NOISE_ENABLED "Enables use of libnoise" OFF)

set(SRC_LIST stdafx.cpp octreeapp.cpp octreebuilder.cpp entity.cpp)

if(NOISE_ENABLED)
    find_library(NOISE_LIBRARY NAMES noise)
    if(NOT NOISE_LIBRARY)
        message(FATAL_ERROR "NOISE_ENABLED is set but libnoise was not found")
    endif()
    add_definitions(-DNOISE_ENABLED=1)
    list(APPEND SRC_LIST noiseutils.cpp)
endif()

add_executable(octree ${SRC_LIST})

target_link_libraries(octree ${DUKAT_EXAMPLE_LIBS} ${NOISE_LIBRARY})
