include_directories(../../src)

add_executable(sprites stdafx.cpp spritesapp.cpp)
target_link_libraries(sprites dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB})
