# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)

# configure import libs
set(dist_dir ${CMAKE_SOURCE_DIR}/../../../../external/distribution)

add_library( SDL2 SHARED IMPORTED )
set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION
		${dist_dir}/lib/${ANDROID_ABI}/libSDL2.so)

add_library( SDL2_image SHARED IMPORTED )
set_target_properties(SDL2_image PROPERTIES IMPORTED_LOCATION
		${dist_dir}/lib/${ANDROID_ABI}/libSDL2_image.so)

add_library(dukat STATIC IMPORTED)
set_target_properties(dukat PROPERTIES IMPORTED_LOCATION
		${dist_dir}/lib/${ANDROID_ABI}/libdukat.a)

# build app's main shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
add_library(main SHARED hellodukat.cpp)

target_include_directories(main PRIVATE ${dist_dir}/include/)

find_library(log-lib log)
find_library(android-lib android)
#find_library(gles1-lib GLESv1_CM)
#find_library(gles2-lib GLESv2)
find_library(gles3-lib GLESv3)

target_link_libraries(main
        dukat
        SDL2
        SDL2_image
		#${gles1-lib} ${gles2-lib}
		${gles3-lib}
		${log-lib}
		${android-lib})
