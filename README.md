# Overview

## Dependencies
* Boost 1.58.0
* Box2D 2.3.1
* glew 2.1.0
* LibPNG 1.6.16
* Robin Hood 3.9.1
* SDL2 2.0.8
* SDL2_image 2.0.3
* SDL2_mixer 2.0.2
* spdlog 1.5.0
* libcurl 8.12.0

# Installation

## Windows
Place header files for the following libraries in include/ directory:
* / LibPNG
* / Robin Hood
* GL/ OpenGL Extension Wranger Library
* SDL2/ SDL 2
* SDL2/ SDL Image 2
* SDL2/ SDL Mixer 2
* spdlog/ spdlog
* curl/ CURL

Set BOOST_PATH environment variable to the top-level directory of your boost install (ex: c:\boost_1_58_0).

Place the following static library files in the lib/x86 or lib/x64 directory:
* Box2D.lib
* glew32.lib
* libpng16.lib
* SDL2.lib
* SDL2_image.lib
* SDL2_mixer.lib
* SDL2main.lib
* libcurl.lib

# Usage

## Windows
Place the following runtime libraries files in the bin/x86 or lib/x64 directory to run the examples:
* glew32.dll
* libcurl.dll
* libFLAC-8.dll
* libjpeg-9.dll
* libmodplug-1.dll
* libmpg123-0.dll
* libogg-0.dll
* libpng16-16.dll
* libtiff-5.dll
* libvorbis-0.dll
* libvorbisfile-3.dll
* libwebp-4.dll
* SDL2.dll
* SDL2_image.dll
* SDL2_mixer.dll
* zlib1.dll

