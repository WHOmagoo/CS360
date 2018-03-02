cmake_minimum_required(VERSION 3.9)
project(SHsimulator C ASM)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_COMPILER gcc)
set(CMAKE_CXX_FLAGS -m32)
set(CMAKE_ASM_FLAGS -m32)

set(SOURCES client.c)

add_executable(Client ${SOURCES})

add_executable(Server server.c)