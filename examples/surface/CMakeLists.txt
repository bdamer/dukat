include_directories(../../src)

add_executable(surface stdafx.cpp surfaceapp.cpp)
target_link_libraries(surface dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB})
