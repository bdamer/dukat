include_directories(../../include)

add_executable(scrolling stdafx.cpp scrollingapp.cpp)
target_link_libraries(scrolling ${DUKAT_EXAMPLE_LIBS})
