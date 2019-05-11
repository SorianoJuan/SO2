cmake_minimum_required(VERSION 3.13)

enable_language(C)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lnetcdf")

project(TP2)

add_executable(main
        src/main.c)

add_custom_target(TP2 COMMAND make -C ${TP2_SOURCE_DIR}
        CLION_EXE_DIR=${build})

target_link_libraries (main netcdf)
