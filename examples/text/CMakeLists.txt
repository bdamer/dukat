include_directories(../../src)

add_executable(text stdafx.cpp text.cpp)
target_link_libraries(text dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB})
