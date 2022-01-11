include_directories(../../include)

add_executable(flocking stdafx.cpp flockingapp.cpp boid.cpp)
target_link_libraries(flocking dukat ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES}
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB} ${CMAKE_THREAD_LIBS_INIT})
