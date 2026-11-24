include_directories(../../include)

add_executable(lighting2d stdafx.cpp lighting2dapp.cpp)
target_link_libraries(lighting2d ${DUKAT_EXAMPLE_LIBS})
