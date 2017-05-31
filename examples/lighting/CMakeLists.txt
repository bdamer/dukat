include_directories(../../src)

add_executable(lighting stdafx.cpp lightingapp.cpp)
target_link_libraries(lighting dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES}
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB})
