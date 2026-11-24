include_directories(../../include)

add_executable(text stdafx.cpp text.cpp)
target_link_libraries(text ${DUKAT_EXAMPLE_LIBS})
