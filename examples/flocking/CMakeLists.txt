include_directories(../../src)

add_executable(flocking stdafx.cpp flockingapp.cpp boid.cpp)
target_link_libraries(flocking dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB})
