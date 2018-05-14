cmake_minimum_required (VERSION 3.0)

enable_language(C)
FIND_PACKAGE(Threads)

SET(CMAKE_CXX_FLAGS -pthread)


project (TP1)


add_executable(server
        src/server/main.c)

add_executable(cliente
        src/client/main.c)

target_link_libraries(cliente pthread)

