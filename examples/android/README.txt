Well, this is quite painful

- SDL2 needs to be built + copied to distribution/lib
- SDL2_image depends on SDL2, had to add local lib:
LOCAL_LDLIBS := ../../external/SDL2/build/intermediates/ndkBuild/debug/obj/local/$(TARGET_ARCH_ABI)/libSDL2.so

comment out
# LOCAL_SHARED_LIBRARIES := SDL2


add includes:
LOCAL_C_INCLUDES += ../../external/distribution/include/SDL2


apply patch: https://github.com/julienr/libpng-android/issues/6

Use copy-libs script

