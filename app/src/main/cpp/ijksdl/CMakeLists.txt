cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)

set(libs_DIR ${CMAKE_SOURCE_DIR}/../../../../distribution)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

add_library(ijkffmpeg SHARED IMPORTED)
set_target_properties(ijkffmpeg PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/ffmpeg/libijkffmpeg.so)

add_library(yuv_static STATIC IMPORTED)
set_target_properties(yuv_static PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/static_yuv/libyuv_static.a)

add_library(ijkj4a STATIC IMPORTED)
set_target_properties(ijkj4a PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/ijkj4a/libijkj4a.a)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

add_library(ijksdl SHARED
            ijksdl_aout.c
            ijksdl_audio.c
            ijksdl_egl.c
            ijksdl_error.c
            ijksdl_mutex.c
            ijksdl_stdinc.c
            ijksdl_thread.c
            ijksdl_timer.c
            ijksdl_vout.c
            ijksdl_extra_log.c
            gles2/color.c
            gles2/common.c
            gles2/renderer.c
            gles2/renderer_rgb.c
            gles2/renderer_yuv420p.c
            gles2/renderer_yuv444p10le.c
            gles2/shader.c
            gles2/fsh/rgb.fsh.c
            gles2/fsh/yuv420p.fsh.c
            gles2/fsh/yuv444p10le.fsh.c
            gles2/vsh/mvp.vsh.c

            dummy/ijksdl_vout_dummy.c

            ffmpeg/ijksdl_vout_overlay_ffmpeg.c
            ffmpeg/abi_all/image_convert.c

            android/android_audiotrack.c
            android/android_nativewindow.c
            android/ijksdl_android_jni.c
            android/ijksdl_aout_android_audiotrack.c
            android/ijksdl_aout_android_opensles.c
            android/ijksdl_codec_android_mediacodec_dummy.c
            android/ijksdl_codec_android_mediacodec_internal.c
            android/ijksdl_codec_android_mediacodec_java.c
            android/ijksdl_codec_android_mediacodec.c
            android/ijksdl_codec_android_mediadef.c
            android/ijksdl_codec_android_mediaformat_java.c
            android/ijksdl_codec_android_mediaformat.c
            android/ijksdl_vout_android_nativewindow.c
            android/ijksdl_vout_android_surface.c
            android/ijksdl_vout_overlay_android_mediacodec.c)

target_include_directories(ijksdl PRIVATE
                           ${CMAKE_SOURCE_DIR}/ijkffmpeg
                           ${CMAKE_SOURCE_DIR}
                           include/ijkj4a/j4a
                           include/ijkj4a/j4au/class/android/media
                           include/ijkj4a/j4au/class/java/nio
                           include/ijkj4a/j4a/class/tv/danmaku/ijk/media/player
                           include/ijkj4a/j4a/class/tv/danmaku/ijk/media/player/misc
                           include/ijkj4a/misc
                           include/ijkj4a/j4a/class/android/os
                           include/ijkj4a/j4a/class/android/media
                           include/ijkj4a/j4a/class/java/util
                           include/ijkj4a/j4a/class/java/nio
                           include/ijkyuv)

include(AndroidNdkModules)
android_ndk_import_module_cpufeatures()

target_link_libraries(ijksdl
                      ijkffmpeg
                      cpufeatures
                      yuv_static
                      ijkj4a
                      OpenSLES
                      EGL
                      GLESv2
                      android
                      log)