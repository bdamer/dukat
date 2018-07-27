include_directories(../../src)

add_executable(terrain stdafx.cpp terrainapp.cpp)
target_link_libraries(terrain dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${PNG_LIBRARY}
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB} ${CMAKE_THREAD_LIBS_INIT})
