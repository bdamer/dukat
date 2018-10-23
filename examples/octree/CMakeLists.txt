include_directories(../../include)

option(NOISE_ENABLED "Enables use of libnoise" OFF)

# Libraries
if (NOISE_ENABLED)
    # FIXME
    set(NOISE_LIBRARY /usr/lib/libnoise.so.0)
    add_definitions(-DNOISE_ENABLED=1)
endif()

set(SRC_LIST stdafx.cpp octreeapp.cpp octreebuilder.cpp entity.cpp)

if(NOISE_ENABLED)
    list(APPEND SRC_LIST noiseutils.cpp)
endif(NOISE_ENABLED)

add_executable(octree ${SRC_LIST})

target_link_libraries(octree dukat 
    ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} 
    ${X11_Xext_LIB} ${CMAKE_THREAD_LIBS_INIT} ${NOISE_LIBRARY}
    )
