cmake_minimum_required(VERSION 3.16.3)

project(FFMPEG_H264_ENCODE)

include_directories(/usr/local/ffmpeg/include)
link_directories(/usr/local/ffmpeg/lib)
set(ENCODE_SRC main.cpp)

add_executable(ENCODE_FFMPEG ${ENCODE_SRC})
target_link_libraries(ENCODE_FFMPEG -lavcodec -lavformat -lavutil -lpthread -lz -lswscale -lswresample -lx264 -ldl)