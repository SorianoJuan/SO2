cmake_minimum_required(VERSION 3.13)

enable_language(C)
FIND_PACKAGE(Threads)
set(CMAKE_CXX_STANDARD 14)

project(TP1)

add_executable(server
        src/server/main.c)

add_executable(cliente
        src/client/main.c)

add_custom_target(TP1 COMMAND make -C ${TP1_SOURCE_DIR}
        CLION_EXE_DIR=${build})

target_link_libraries(server pthread)
