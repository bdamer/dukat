include_directories(../../src)

add_executable(mapgen stdafx.cpp mapgenapp.cpp mapgraph.cpp voronoi.cpp)
# voronoi.cpp rb.c
target_link_libraries(mapgen dukat ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} 
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_Xext_LIB})
