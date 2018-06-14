cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project("LAStools")

add_compile_options(-O3 -Wall -Wno-strict-aliasing)

add_subdirectory(LASlib/src)
add_subdirectory(LASlib/example)
add_subdirectory(LASzip/example)
add_subdirectory(src)

