cmake_minimum_required(VERSION 3.11)
project(libjpeg)

aux_source_directory(src SRC)

add_library(jpeg_port ${SRC})
add_executable(djpeg test/jpegtest.c)
target_link_libraries(djpeg jpeg_port)
