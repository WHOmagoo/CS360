cmake_minimum_required(VERSION 3.9)
project(CS360Lab2 C ASM)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_COMPILER gcc)
set(CMAKE_CXX_FLAGS -m32)
set(CMAKE_ASM_FLAGS -m32)

set(SOURCES t.c s.s queue.o)


add_executable(Cur ${SOURCES})
#add_custom_target(run COMMAND make -f mk)
#add_custom_target(run COMMAND binary DEPENDS run WORKING_DIRECTORY $(CMAKE_PROJECT_DIR))