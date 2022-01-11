include_directories(../../include)

add_executable(room stdafx.cpp roomapp.cpp roomcamera.cpp)
target_link_libraries(room dukat ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB} ${CMAKE_THREAD_LIBS_INIT})
