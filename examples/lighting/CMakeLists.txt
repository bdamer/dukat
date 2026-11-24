include_directories(../../include)

add_executable(lighting stdafx.cpp lightingapp.cpp)
target_link_libraries(lighting ${DUKAT_EXAMPLE_LIBS})
