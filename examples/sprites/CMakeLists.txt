include_directories(../../include)

add_executable(sprites stdafx.cpp spritesapp.cpp)
target_link_libraries(sprites ${DUKAT_EXAMPLE_LIBS})
