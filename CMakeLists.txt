cmake_minimum_required(VERSION 3.29)
project(HPCProject1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Add the executable target
add_executable(HPCProject1
        main.c
        sparsematrix.h
        sparsematrix.c
        logs.c
        matrices.c
        matrices.h
)

# Link OpenMP to your target
target_link_libraries(HPCProject1 PRIVATE OpenMP::OpenMP_C)

# If you want to use OpenMP flags for compilation
if(OpenMP_C_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif()