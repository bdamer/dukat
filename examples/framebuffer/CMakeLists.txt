include_directories(../../src)

add_executable(framebuffer stdafx.cpp framebufferapp.cpp)
target_link_libraries(framebuffer dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB})
