include_directories(../../include)

add_executable(hello stdafx.cpp hellodukat.cpp)
target_link_libraries(hello ${DUKAT_EXAMPLE_LIBS})
