cmake_minimum_required(VERSION 3.10)

project(raytrace)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable( raytrace ppm-image.cpp)
