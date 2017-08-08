include_directories(../../src)

add_executable(waves stdafx.cpp wavesapp.cpp)
target_link_libraries(waves dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB})
