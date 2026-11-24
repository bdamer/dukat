include_directories(../../include)

add_executable(layers stdafx.cpp layersapp.cpp)
target_link_libraries(layers ${DUKAT_EXAMPLE_LIBS})
