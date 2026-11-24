include_directories(../../include)

add_executable(waves stdafx.cpp wavesapp.cpp)
target_link_libraries(waves ${DUKAT_EXAMPLE_LIBS})
