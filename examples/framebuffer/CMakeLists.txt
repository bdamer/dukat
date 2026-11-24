include_directories(../../include)

add_executable(framebuffer stdafx.cpp framebufferapp.cpp causticsscene.cpp fractalscene.cpp ripplepondscene.cpp titlescene.cpp)
target_link_libraries(framebuffer ${DUKAT_EXAMPLE_LIBS})
