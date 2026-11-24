include_directories(../../include)

add_executable(scenes stdafx.cpp scenes.cpp gamescene.cpp titlescene.cpp dialogscene.cpp)
target_link_libraries(scenes ${DUKAT_EXAMPLE_LIBS})
