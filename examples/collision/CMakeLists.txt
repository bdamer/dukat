include_directories(../../include)

add_executable(collision stdafx.cpp collisionapp.cpp)
target_link_libraries(collision ${DUKAT_EXAMPLE_LIBS})
