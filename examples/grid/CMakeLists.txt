include_directories(../../include)

add_executable(grid stdafx.cpp gridapp.cpp)
target_link_libraries(grid ${DUKAT_EXAMPLE_LIBS} ${PNG_LIBRARIES})
