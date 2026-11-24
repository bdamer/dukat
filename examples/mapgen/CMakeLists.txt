include_directories(../../include)

add_executable(mapgen stdafx.cpp mapgenapp.cpp)
target_link_libraries(mapgen ${DUKAT_EXAMPLE_LIBS})
