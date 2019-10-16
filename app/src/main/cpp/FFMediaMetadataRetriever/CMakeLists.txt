cmake_minimum_required(VERSION 3.4.1)

set(libs_DIR ${CMAKE_SOURCE_DIR}/../../../../distribution)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

add_library(ijkffmpeg SHARED IMPORTED)
set_target_properties(ijkffmpeg PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/ffmpeg/libijkffmpeg.so)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=soft")

add_library(ffmpeg_mediametadataretriever_jni SHARED
                    wseemann_media_MediaMetadataRetriever.cpp
                    mediametadataretriever.cpp
                    ffmpeg_mediametadataretriever.c
                    ffmpeg_utils.c)

target_include_directories(ffmpeg_mediametadataretriever_jni PRIVATE
                           ${CMAKE_SOURCE_DIR}/ijkffmpeg )

target_link_libraries(ffmpeg_mediametadataretriever_jni
                      ijkffmpeg
                      jnigraphics
                      android
                      log)
