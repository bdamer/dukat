include_directories(../../src)

add_executable(scenes stdafx.cpp scenes.cpp gamescene.cpp textbutton.cpp titlescene.cpp dialogscene.cpp)
target_link_libraries(scenes dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES}
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB})
