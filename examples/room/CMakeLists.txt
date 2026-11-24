include_directories(../../include)

add_executable(room stdafx.cpp roomapp.cpp roomcamera.cpp)
target_link_libraries(room ${DUKAT_EXAMPLE_LIBS})
