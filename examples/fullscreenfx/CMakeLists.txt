include_directories(../../include)

add_executable(fullscreenfx stdafx.cpp fullscreenfxapp.cpp)
target_link_libraries(fullscreenfx ${DUKAT_EXAMPLE_LIBS})
