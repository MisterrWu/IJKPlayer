#
# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Build a static lib and copy it into distribution place for independent app to use
# we could not delete the bin directory is because android studio would error out
# when it checks target's existance. Only distribution place is used as import to app

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)

add_library(ijkj4a STATIC
                                  j4a/j4a_allclasses.c
                                  j4a/j4a_base.c
                                  j4a/class/android/media/AudioTrack.c
                                  j4a/class/android/media/MediaCodec.c
                                  j4a/class/android/media/MediaFormat.c
                                  j4a/class/android/media/PlaybackParams.c
                                  j4a/class/android/os/Build.c
                                  j4a/class/android/os/Bundle.c
                                  j4a/class/java/nio/Buffer.c
                                  j4a/class/java/nio/ByteBuffer.c
                                  j4a/class/java/util/ArrayList.c
                                  j4a/class/tv/danmaku/ijk/media/player/misc/IMediaDataSource.c
                                  j4a/class/tv/danmaku/ijk/media/player/misc/IAndroidIO.c
                                  j4a/class/tv/danmaku/ijk/media/player/IjkMediaPlayer.c

                                  j4au/class/android/media/AudioTrack.util.c
                                  j4au/class/java/nio/ByteBuffer.util.c )

target_include_directories(ijkj4a PRIVATE
                                  j4a
                                  j4a/class/android/media
                                  j4a/class/android/os
                                  j4a/class/java/nio
                                  j4a/class/java/util
                                  j4a/class/tv/danmaku/ijk/media/player/misc
                                  j4a/class/tv/danmaku/ijk/media/player

                                  j4au/class/android/media
                                  j4au/class/java/nio )

# copy out the lib binary... need to leave the static lib around to pass gradle check
set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../distribution)
set_target_properties(ijkj4a
                      PROPERTIES
                      ARCHIVE_OUTPUT_DIRECTORY
                      "${distribution_DIR}/ijkj4a")

# copy out lib header file...
add_custom_command(TARGET ijkj4a POST_BUILD
#                   拷贝文件
#                   COMMAND "${CMAKE_COMMAND}" -E
#                   copy "${CMAKE_CURRENT_SOURCE_DIR}/src/soundtouch.h"
#                   "${distribution_DIR}/soundtouch/include/soundtouch.h"
#                   *************copy_directory*************
#                  拷贝路径
#                  COMMAND "${CMAKE_COMMAND}" -E
#                  copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src"
#                  "${distribution_DIR}/gmath/include"
#                   **** the following 2 lines are for potential future debug purpose ****
#                   COMMAND "${CMAKE_COMMAND}" -E
#                   remove_directory "${CMAKE_CURRENT_BINARY_DIR}"
                   COMMENT "Copying ijkj4a to output directory")

