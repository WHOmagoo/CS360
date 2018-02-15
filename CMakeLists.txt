cmake_minimum_required(VERSION 3.9)
project(SHsimulator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_COMPILER gcc)
set(CMAKE_CXX_FLAGS -m32)

set(SOURCES sh.c)

add_executable(CS360 ${SOURCES})