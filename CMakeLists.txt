cmake_minimum_required(VERSION 3.29)
project(HPCProject1 C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
set(CMAKE_C_STANDARD 11)

add_executable(HPCProject1 main.c
        sparsematrix.h
        sparsematrix.c
        logs.c
        matrices.c
        matrices.h)
