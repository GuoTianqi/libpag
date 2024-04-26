cmake_minimum_required(VERSION 3.5)
project(pag-linux)
set(CMAKE_CXX_STANDARD 17)

if (APPLE AND NOT IOS)
    SET(CMAKE_SYSTEM_PROCESSOR x86_64)
    SET(CMAKE_OSX_ARCHITECTURES x86_64)
endif ()

add_definitions(-DSWIFTSHADER)
add_definitions(-DGL_GLEXT_PROTOTYPES)
add_definitions(-DEGL_EGLEXT_PROTOTYPES)

add_executable(pag-linux src/main.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/libpag/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_build/include # ffmpeg
    src)

if (APPLE)
    find_library(APPLICATION_SERVICES_FRAMEWORK ApplicationServices REQUIRED)
    list(APPEND libs ${APPLICATION_SERVICES_FRAMEWORK})
    find_library(QUARTZ_CORE QuartzCore REQUIRED)
    list(APPEND libs ${QUARTZ_CORE})
    find_library(COCOA Cocoa REQUIRED)
    list(APPEND libs ${COCOA})
    find_library(FOUNDATION Foundation REQUIRED)
    list(APPEND libs ${FOUNDATION})
    find_library(VIDEOTOOLBOX VideoToolbox)
    list(APPEND libs ${VIDEOTOOLBOX})
    find_library(COMPRESSION_LIBRARIES NAMES compression)
    list(APPEND libs ${COMPRESSION_LIBRARIES})

    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libpag/mac/x64/libpag.a)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/vendor/swiftshader/mac/x64/libEGL.dylib)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/vendor/swiftshader/mac/x64/libGLESv2.dylib)

else ()

    message("===============: linux")
    message("===============: ${CMAKE_CURRENT_SOURCE_DIR}")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cppFlags} -pthread")
    add_compile_options(-fPIC)
    add_definitions(-D__STDC_FORMAT_MACROS)
    SET(CMAKE_EXE_LINKER_FLAGS " -no-pie")

    find_package(Threads)
    list(APPEND libs ${CMAKE_THREAD_LIBS_INIT})
    find_package(ZLIB)
    include_directories(${ZLIB_INCLUDE_DIRS})
    list(APPEND libs ${ZLIB_LIBRARIES})
    list(APPEND libs dl)

    # ffmpeg: libavcodec.a  libavdevice.a  libavfilter.a  libavformat.a  libavutil.a  libpostproc.a  libswresample.a  libswscale.a

    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_build/lib/libavdevice.a)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_build/lib/libavfilter.a)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_build/lib/libavformat.a)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_build/lib/libavcodec.a)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_build/lib/libavutil.a)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_build/lib/libswresample.a)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_build/lib/libswscale.a)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_build/lib/libpostproc.a)


    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libpag/linux/x64/libpag.a)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/vendor/swiftshader/linux/x64/libEGL.so)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/vendor/swiftshader/linux/x64/libGLESv2.so)

    message("===============: ${libs}")
endif ()

target_link_libraries(pag-linux
        ${libs}
        m atomic lzma z mp3lame x264 pthread dl va bz2 gnutls gmp unistring atomic hogweed nettle tasn1 idn2 p11-kit
        va-drm va-x11 X11 Xv Xext vdpau
        )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g -O1")
endif ()

