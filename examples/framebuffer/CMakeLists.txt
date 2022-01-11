include_directories(../../include)

add_executable(framebuffer stdafx.cpp framebufferapp.cpp fractalscene.cpp ripplepondscene.cpp titlescene.cpp)
target_link_libraries(framebuffer dukat ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB} ${CMAKE_THREAD_LIBS_INIT})
