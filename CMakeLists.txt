cmake_minimum_required(VERSION 3.9)
project(CS360Lab2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_COMPILER gcc)
set(CMAKE_CXX_FLAGS -m3)

add_executable(CS360 queue.c)