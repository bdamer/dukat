include_directories(../../include)

add_executable(surface stdafx.cpp surfaceapp.cpp)
target_link_libraries(surface ${DUKAT_EXAMPLE_LIBS})
