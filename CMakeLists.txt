cmake_minimum_required(VERSION 3.9)
project(SHsimulator C ASM)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_COMPILER gcc)
set(CMAKE_CXX_FLAGS -m32)
set(CMAKE_ASM_FLAGS -m32)

set(SOURCES t.c type.h s.s queue.o sh.c)

add_executable(SHsimulator ${SOURCES})
#add_custom_target(run COMMAND make -f mk)
#add_custom_target(run COMMAND binary DEPENDS run WORKING_DIRECTORY $(CMAKE_PROJECT_DIR))