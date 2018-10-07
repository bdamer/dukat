include_directories(../../src)

add_executable(layers stdafx.cpp layersapp.cpp)
target_link_libraries(layers dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB} ${CMAKE_THREAD_LIBS_INIT})
