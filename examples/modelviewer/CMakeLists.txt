include_directories(../../src)

add_executable(modelviewer stdafx.cpp modelviewer.cpp model3.cpp ms3dmodel.cpp)
target_link_libraries(modelviewer dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${PNG_LIBRARY}
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB})
