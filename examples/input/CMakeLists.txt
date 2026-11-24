include_directories(../../include)

add_executable(input stdafx.cpp inputapp.cpp)
target_link_libraries(input ${DUKAT_EXAMPLE_LIBS})
