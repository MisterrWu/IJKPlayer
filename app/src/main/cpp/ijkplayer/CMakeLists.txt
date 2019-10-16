cmake_minimum_required(VERSION 3.4.1)

set(libs_DIR ${CMAKE_SOURCE_DIR}/../../../../distribution)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

add_library(ijkffmpeg SHARED IMPORTED)
set_target_properties(ijkffmpeg PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/ffmpeg/libijkffmpeg.so)

add_library(android-ndk-profiler STATIC IMPORTED)
set_target_properties(android-ndk-profiler PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/ijkprof/libandroid-ndk-profiler.a)

add_library(ijksoundtouch STATIC IMPORTED)
set_target_properties(ijksoundtouch PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/soundtouch/libijksoundtouch.a)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=soft")

add_library(ijkplayer SHARED
            ff_cmdutils.c
            ff_ffplay.c
            ff_ffpipeline.c
            ff_ffpipenode.c
            ijkmeta.c
            ijkplayer.c

            pipeline/ffpipeline_ffplay.c
            pipeline/ffpipenode_ffplay_vdec.c

            android/ffmpeg_api_jni.c
            android/ijkplayer_android.c
            android/ijkplayer_jni.c

            android/pipeline/ffpipeline_android.c
            android/pipeline/ffpipenode_android_mediacodec_vdec.c

            ijkavformat/allformats.c
            ijkavformat/ijklivehook.c
            ijkavformat/ijkmediadatasource.c
            ijkavformat/ijkio.c
            ijkavformat/ijkiomanager.c
            ijkavformat/ijkiocache.c
            ijkavformat/ijkioffio.c
            ijkavformat/ijkioandroidio.c
            ijkavformat/ijkioprotocol.c
            ijkavformat/ijkioapplication.c
            ijkavformat/ijkiourlhook.c

            ijkavformat/ijkasync.c
            ijkavformat/ijkurlhook.c
            ijkavformat/ijklongurl.c
            ijkavformat/ijksegment.c

            ijkavutil/ijkdict.c
            ijkavutil/ijkutils.c
            ijkavutil/ijkthreadpool.c
            ijkavutil/ijktree.c
            ijkavutil/ijkfifo.c
            ijkavutil/ijkstl.cpp)

target_include_directories(ijkplayer PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_SOURCE_DIR}
                           ${CMAKE_SOURCE_DIR}/ijkffmpeg
                           include/ijksoundtouch
                           include/ijkj4a/j4a
                           include/ijkj4a/j4au/class/android/media
                           include/ijkj4a/j4au/class/java/nio
                           include/ijkj4a/j4a/class/tv/danmaku/ijk/media/player
                           include/ijkj4a/j4a/class/tv/danmaku/ijk/media/player/misc
                           include/ijkj4a/misc
                           include/ijkj4a/j4a/class/android/os
                           include/ijkj4a/j4a/class/android/media
                           include/ijkj4a/j4a/class/java/util
                           include/ijkj4a/j4a/class/java/nio)

target_link_libraries(ijkplayer
                      ijkffmpeg
                      ijksdl
                      android-ndk-profiler
                      ijksoundtouch
                      android
                      log)