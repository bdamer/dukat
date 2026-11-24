include_directories(../../include)

add_executable(flocking stdafx.cpp flockingapp.cpp boid.cpp)
target_link_libraries(flocking ${DUKAT_EXAMPLE_LIBS})
