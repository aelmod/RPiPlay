cmake_minimum_required(VERSION 3.15)

add_subdirectory(h264-bitstream)

set(Boost_COMPILER mgw81)
find_package(Boost REQUIRED COMPONENTS system serialization)
include_directories(${Boost_INCLUDE_DIRS})

add_library(renderers
        STATIC
        audio_renderer_dummy.c video_renderer_dummy.cpp h264_data.h #[[decode_video.c]])
target_link_libraries(renderers h264-bitstream ${Boost_LIBRARIES})
