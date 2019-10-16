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

add_library(ijksoundtouch STATIC
                              SoundTouch/AAFilter.cpp
                              SoundTouch/FIFOSampleBuffer.cpp
                              SoundTouch/FIRFilter.cpp
                              SoundTouch/cpu_detect_x86.cpp
                              SoundTouch/sse_optimized.cpp
                              SoundTouch/RateTransposer.cpp
                              SoundTouch/SoundTouch.cpp
                              SoundTouch/InterpolateCubic.cpp
                              SoundTouch/InterpolateLinear.cpp
                              SoundTouch/InterpolateShannon.cpp
                              SoundTouch/TDStretch.cpp
                              SoundTouch/BPMDetect.cpp
                              SoundTouch/PeakFinder.cpp
                              SoundTouch/mmx_optimized.cpp
                              ijksoundtouch_wrap.cpp)

target_include_directories(ijksoundtouch PRIVATE
                           include)

# copy out the lib binary... need to leave the static lib around to pass gradle check
set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../distribution)
set_target_properties(ijksoundtouch
                      PROPERTIES
                      ARCHIVE_OUTPUT_DIRECTORY
                      "${distribution_DIR}/soundtouch")

# copy out lib header file...
add_custom_command(TARGET ijksoundtouch POST_BUILD
#                   *************copy_directory*************
#                  拷贝路径
#                  COMMAND "${CMAKE_COMMAND}" -E
#                  copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src"
#                  "${distribution_DIR}/gmath/include"
                   COMMENT "Copying ijksoundtouch to output directory")

