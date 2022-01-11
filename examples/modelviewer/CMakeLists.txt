include_directories(../../include)

add_executable(modelviewer stdafx.cpp modelviewer.cpp)
target_link_libraries(modelviewer dukat ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${PNG_LIBRARY}
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB} ${CMAKE_THREAD_LIBS_INIT})
