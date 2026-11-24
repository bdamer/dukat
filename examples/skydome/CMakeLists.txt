include_directories(../../include)

add_executable(skydome stdafx.cpp skydomeapp.cpp)
target_link_libraries(skydome ${DUKAT_EXAMPLE_LIBS})
