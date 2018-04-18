include_directories(../../src)

add_executable(collision stdafx.cpp collisionapp.cpp)
target_link_libraries(collision dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB})
