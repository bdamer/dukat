cmake_minimum_required(VERSION 3.4.1)
include_directories(../include)

# Define sources
file(GLOB SOURCE_FILES *.cpp)
if(ANDROID)
	file (GLOB SOURCE_FILES
		aabb2.cpp aabb3.cpp animationmanager.cpp animationsequence.cpp application.cpp assetloader.cpp
		bit.cpp bitmapfont.cpp blockbuilder.cpp boundingcircle.cpp boundingsphere.cpp buffers.cpp
		camera2.cpp camera3.cpp causticseffect2.cpp collisionmanager2.cpp color.cpp
		debugeffect2.cpp devicemanager.cpp dither.cpp draw.cpp
		effectpass.cpp environment.cpp eulerangles.cpp
		feedback.cpp firstpersoncamera3.cpp fixedcamera3.cpp fontcache.cpp fullscreeneffect2.cpp 
		game2.cpp game3.cpp gamebase.cpp gamepaddevice.cpp geometry.cpp
		inputdevice.cpp inputrecorder.cpp json.cpp keyboarddevice.cpp log.cpp mathutil.cpp matrix2.cpp matrix4.cpp meshbuilder2.cpp meshbuilder3.cpp
		meshcache.cpp meshdata.cpp meshgroup.cpp meshinstance.cpp messenger.cpp model3.cpp obb2.cpp orbitcamera3.cpp 
		particle.cpp particleemitter.cpp particlemanager.cpp perfcounter.cpp quaternion.cpp
		rand.cpp ray3.cpp renderer.cpp renderer2.cpp renderer3.cpp renderlayer2.cpp 
		scene2.cpp sdlutil.cpp settings.cpp shadercache.cpp shaderprogram.cpp shadoweffect2.cpp sprite.cpp
		shakycameraeffect.cpp stdafx.cpp string.cpp surface.cpp sysutil.cpp
		textmeshbuilder.cpp textmeshinstance.cpp texturecache.cpp texture.cpp textureutil.cpp timermanager.cpp transform3.cpp 
		uimanager.cpp vector2.cpp vector3.cpp window.cpp)
endif()

add_library(dukat STATIC ${SOURCE_FILES})

# Define archive dir
set(archive_dir "${CMAKE_CURRENT_SOURCE_DIR}/../lib")
if(ANDROID)
	set(target_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../..)
	set(archive_dir "${target_dir}/distribution/lib/${ANDROID_ABI}")
	target_include_directories(dukat PRIVATE "${target_dir}/distribution/include")
	# copy out public headers
	file(GLOB MY_PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/../include/dukat/*.h")
	file(COPY ${MY_PUBLIC_HEADERS} DESTINATION "${target_dir}/distribution/include/dukat")
endif()
set_target_properties(dukat PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${archive_dir})
