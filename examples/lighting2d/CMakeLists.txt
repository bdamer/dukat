include_directories(../../include)

add_executable(lighting2d stdafx.cpp lighting2dapp.cpp)
target_link_libraries(lighting2d dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB} ${CMAKE_THREAD_LIBS_INIT})
