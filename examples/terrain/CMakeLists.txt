include_directories(../../include)

add_executable(terrain stdafx.cpp terrainapp.cpp)
target_link_libraries(terrain ${DUKAT_EXAMPLE_LIBS} ${PNG_LIBRARIES})
