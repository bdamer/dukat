include_directories(../../include)

add_executable(particles stdafx.cpp particles.cpp)
target_link_libraries(particles ${DUKAT_EXAMPLE_LIBS})
