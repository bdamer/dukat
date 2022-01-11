include_directories(../../include)

add_executable(scenes stdafx.cpp scenes.cpp gamescene.cpp titlescene.cpp dialogscene.cpp)
target_link_libraries(scenes dukat ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES}
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB} ${CMAKE_THREAD_LIBS_INIT})
