include_directories(../../include)

add_executable(heatmap stdafx.cpp heatmapapp.cpp heatmap.cpp)
target_link_libraries(heatmap ${DUKAT_EXAMPLE_LIBS} ${PNG_LIBRARIES})
