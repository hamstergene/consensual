cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(runtests)

add_executable(runtests
    src/runtime.c
    tests/runtime_tests.c
    tests/main.c
    )

target_link_libraries(runtests check)

include_directories(runtests
    include
    )

