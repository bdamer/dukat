include_directories(../../include)

add_executable(modelviewer stdafx.cpp modelviewer.cpp)
target_link_libraries(modelviewer ${DUKAT_EXAMPLE_LIBS} ${PNG_LIBRARIES})
