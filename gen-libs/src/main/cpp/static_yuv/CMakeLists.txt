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

add_library(yuv_static STATIC
                                  source/compare.cc
                                  source/compare_common.cc
                                  source/compare_posix.cc
                                  source/convert.cc
                                  source/convert_argb.cc
                                  source/convert_from.cc
                                  source/convert_from_argb.cc
                                  source/convert_to_argb.cc
                                  source/convert_to_i420.cc
                                  source/cpu_id.cc
                                  source/format_conversion.cc
                                  source/planar_functions.cc
                                  source/rotate.cc
                                  source/rotate_argb.cc
                                  source/rotate_mips.cc
                                  source/row_any.cc
                                  source/row_common.cc
                                  source/row_mips.cc
                                  source/row_posix.cc
                                  source/scale.cc
                                  source/scale_argb.cc
                                  source/scale_common.cc
                                  source/scale_mips.cc
                                  source/scale_posix.cc
                                  source/video_common.cc)

target_include_directories(yuv_static PRIVATE
                           include
                           include/libyuv)

# copy out the lib binary... need to leave the static lib around to pass gradle check
set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../distribution)
set_target_properties(yuv_static
                      PROPERTIES
                      ARCHIVE_OUTPUT_DIRECTORY
                      "${distribution_DIR}/static_yuv")

# copy out lib header file...
add_custom_command(TARGET yuv_static POST_BUILD
#                   *************copy_directory*************
#                  拷贝路径
#                  COMMAND "${CMAKE_COMMAND}" -E
#                  copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src"
#                  "${distribution_DIR}/gmath/include"
                   COMMENT "Copying yuv_static to output directory")

