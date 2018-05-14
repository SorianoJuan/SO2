cmake_minimum_required(VERSION 3.8)
enable_language(C)
project(TP2)

set(CMAKE_CXX_STANDARD 11)

add_executable(procedural
        src/procedural/main.c)

add_executable(openmp
        src/openmp/main.c)

target_link_libraries(procedural m)
target_link_libraries(openmp m)